#include <atomic>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/scope_exit.hpp>
#include <boost/regex.hpp>
#include <boost/range/algorithm.hpp>

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "avif.hpp"
#include "avproto.hpp"
#include "async_coro_queue.hpp"
#include "avim_proto/message.pb.h"

extern const char* avim_root_ca_certificate_string;

template<typename C, typename Pred>
auto container_remove_if_all(C & c, Pred pred) -> decltype(std::begin(c))
{
	auto it = std::begin(c);
	while ((it = std::find_if(it, std::end(c), pred) ) != std::end(c))
	{
		c.erase(it++);
	}
	return it;
}

class root_cert
	: boost::noncopyable
{
	X509_STORE* m_store;
public:
	root_cert(X509* ca)
	{
		m_store = X509_STORE_new();

		X509_STORE_add_cert(m_store, ca);
		X509_STORE_set_default_paths(m_store);
	}

	~root_cert()
	{
		X509_STORE_free(m_store);
	}

	bool verity(X509* cert)
	{
		boost::shared_ptr<X509_STORE_CTX> storeCtx(X509_STORE_CTX_new(), X509_STORE_CTX_free);
		X509_STORE_CTX_init(storeCtx.get(), m_store,cert,NULL);
		X509_STORE_CTX_set_flags(storeCtx.get(), X509_V_FLAG_CB_ISSUER_CHECK);
		return X509_verify_cert(storeCtx.get());
	}

};


class avkernel;
namespace detail
{

typedef boost::shared_ptr<RSA> autoRSAptr;

struct RouteItem
{
	boost::regex pattern;
	std::string gateway;
	std::string ifname;
	int metric;
};

bool operator<(const RouteItem& lhs, const RouteItem& rhs) { return lhs.metric < rhs.metric; }

class avkernel_impl : boost::noncopyable , public boost::enable_shared_from_this<avkernel_impl>
{
	const X509 * const m_root_ca_cert;

	boost::asio::io_service & io_service;
	std::map<std::string, avif> m_avifs;
	std::vector<RouteItem> m_routes;

	struct AVPubKey
	{
		autoRSAptr rsa;
		// valid_until this time
		boost::posix_time::ptime valid_until;
	};

	struct AVdbitem
	{
		std::string avaddr;
		std::vector<AVPubKey> keys;
	};

	std::map<std::string, AVdbitem> trusted_pubkey;

	// 存储接收到的数据包
	boost::async_coro_queue<
		std::queue<
			std::pair<std::string, std::string>
		>
	> m_recv_buffer;

	struct async_wait_packet_pred_handler
	{
		boost::posix_time::ptime deadline;
		boost::function<bool (const proto::avpacket &)> pred;
		boost::function<void (boost::system::error_code)> handler;
	};

	std::list<async_wait_packet_pred_handler> m_async_wait_packet_pred_handler_preprocess_list;
	std::list<async_wait_packet_pred_handler> m_async_wait_packet_pred_handler_postprocess_list;

	bool is_to_me(const proto::av_address & addr)
	{
		// 遍历 interface 做比较
		for (const auto& i : m_avifs)
		{
			if (addr.username() == i.second.if_address()->username() &&
				addr.domain() == i.second.if_address()->domain())
			{
				return true;
			}
		}
		return false;
	}

	// TODO 数据包接收过程的完整实现， 目前只实现个基础的垃圾
	void process_recived_packet_to_me(boost::shared_ptr<proto::avpacket> avPacket, avif avinterface, boost::asio::yield_context yield_context)
	{
		std::cerr << "one pkt from " <<  av_address_to_string(avPacket->src()) << " sended to me" << std::endl;
		std::string add;
		std::string payload;

		// TODO 处理 agmp 协议等等等等

		add = av_address_to_string(avPacket->src());

		avPacket->publickey();

		autoRSAptr rsa(RSA_new(), RSA_free);

		rsa->e = BN_new();
		BN_set_word(rsa->e, 65537);
		rsa->n = BN_bin2bn((const unsigned char*) avPacket->publickey().data(), avPacket->publickey().length(), rsa->n);

		if (avPacket->upperlayerpotocol() == "pkask")
		{
			// 发回 自己的公钥
			return async_send_agmp_pkreply(&avinterface, add);
		}

		if (avPacket->upperlayerpotocol() == "pkreply")
		{
			// TODO 验证并提取公钥，添加到存储中心
			const unsigned char * in = (const unsigned char *) avPacket->payload().data();
			X509 * crt = d2i_X509(0, &in, avPacket->payload().length());
			auto pkey = X509_get_pubkey(crt);
			X509_free(crt);
			RSA * rsa = EVP_PKEY_get1_RSA(pkey);
			EVP_PKEY_free(pkey);

			add_RSA_pubkey(
				av_address_to_string(avPacket->src()),
					rsa,
					boost::posix_time::microsec_clock::local_time() + boost::posix_time::hours(500)
			);
			RSA_free(rsa);
			return ;
		}

		// 有  payload ， 那就一定要解密，呵呵
		if (avPacket->has_payload())
		{
			if (avPacket->upperlayerpotocol() == "agmp")
			{
				payload = avPacket->payload();
			}
			else
			{
				// 第一阶段解密，先使用发送者的公钥解密
				std::string stage1decypted = RSA_public_decrypt(rsa.get(), avPacket->payload());
				// 第二阶段解密，用自己的私钥解密
				payload = RSA_private_decrypt(avinterface.get_rsa_key(), stage1decypted);
			}
		}

		// 挂入本地接收列队，等待上层读取
		if (avPacket->upperlayerpotocol() == "avim")
			m_recv_buffer.push(std::make_pair(add, payload));
	}

	void process_recived_packet(boost::shared_ptr<proto::avpacket> avPacket, avif avinterface, boost::asio::yield_context yield_context)
	{
		BOOST_SCOPE_EXIT_ALL(this, avPacket)
		{
			// 这里是数据包的后处理
			auto it = m_async_wait_packet_pred_handler_postprocess_list.begin();
			while (it != m_async_wait_packet_pred_handler_postprocess_list.end())
			{
				if (it->pred(boost::ref(*avPacket)))
				{
					it->handler(boost::system::error_code());
					m_async_wait_packet_pred_handler_postprocess_list.erase(it++);
				}
				else
				{
					it ++;
				}
			}
		};

		RSA* stored_key = find_RSA_pubkey(av_address_to_string(avPacket->src()));

		// TODO 执行客户端校验
		if (avPacket->has_publickey() && stored_key)
		{
			// 有 pubkey ， 比较内部数据库里的记录
			// 如果和内部数据库的记录不一致
			// 要使用 avgmp 协议请求一份证书来校验一遍

			// 干这种事情的话，就需要开另外一个协程慢慢干了，呵呵
			// 因此加到 TODO 列表

		}
		else if (!stored_key && avPacket->has_publickey())
		{
			// TODO, 执行 证书请求，并在请求通过后，验证数据包接受该

			// 等待  hyq 的校验完成
			// 暂时直接信任然后添加吧
			RSA * rsa = RSA_new();
			rsa->e = BN_new();
			BN_set_word(rsa->e, 65537);
			rsa->n = BN_bin2bn((const unsigned char *) avPacket->publickey().data(), avPacket->publickey().length(), 0);
			add_RSA_pubkey(av_address_to_string(avPacket->src()), rsa, boost::posix_time::microsec_clock::local_time() + boost::posix_time::hours(500));
			RSA_free(rsa);
		}
		else
		{
			 // TODO 执行 证书请求，并在请求通过后，验证数据包接受该
		}

		// 查看据地地址，如果是自己，就交给上层

		if (is_to_me(avPacket->dest()))
		{
			return process_recived_packet_to_me(avPacket, avinterface, yield_context);
		}

		// 查看 ttl
		if (avPacket->time_to_live() <= 1)
		{
			// 丢弃包
			// TODO 向原地址返回一个 ttl 消尽的agmp消息
			return;
		}

		avPacket->set_time_to_live(avPacket->time_to_live() - 1);

		std::cerr << "got one pkt from " << av_address_to_string(avPacket->src())
			<< " to " << av_address_to_string(avPacket->dest());

		// TODO 查找路由表
		avif* interface = select_route(av_address_to_string(avPacket->dest()));

		if (!interface)
		{
			// TODO 返回 no route to host 消息
			std::cerr << "|| but no route to host, dropping packet!!" << std::endl;
			return;
		}

		std::cerr << " , now routing with " << interface->get_ifname() << std::endl;

		// 转发过去
		async_interface_write_packet(interface, avPacket);

		std::cerr << "routed ! " << std::endl;
	}

	void async_recvfrom_op(std::string & target, std::string & data, avkernel::ReadyHandler handler, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;

		auto _data_pair = m_recv_buffer.async_pop(yield_context[ec]);
		if (ec)
		{
			return handler(ec);
		}

		target = std::move(_data_pair.first);
		data = std::move(_data_pair.second);

		handler(ec);
	}

	void async_recvfrom(std::string & target, std::string & data, avkernel::ReadyHandler handler)
	{
		boost::asio::spawn(io_service, boost::bind(&avkernel_impl::async_recvfrom_op, shared_from_this(), boost::ref(target), boost::ref(data), handler, _1 ));
	}

	int recvfrom(std::string & target, std::string &data)
	{
		boost::system::error_code ec;

		boost::mutex m;
		boost::unique_lock< boost::mutex > l(m);
		boost::condition_variable ready;

		async_recvfrom(target, data, [&ec, &ready](const boost::system::error_code & _ec)
			{ ec = _ec; ready.notify_all();});

		ready.wait(l);
		return ec.value();
	}

	// 内部的一个协程循环，用来执行串行发送，保证数据包次序
	void interface_writer(avif avinterface, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;

		while (!(*avinterface.quitting))
		{
			std::pair<avif::auto_avPacketPtr, boost::function<void(boost::system::error_code)>>
				popvalue = avinterface._write_queue->async_pop(yield_context[ec]);
			if (ec)
			{
				break;
			}
			avinterface.async_write_packet(popvalue.first.get(), yield_context[ec]);
			popvalue.second(ec);
		}
		std::cerr << "interface_writer coroutine exited" << std::endl;
	}

	template<class RealHandler>
	void async_interface_write_packet(avif * avinterface, avif::auto_avPacketPtr avPacket, BOOST_ASIO_MOVE_ARG(RealHandler) handler)
	{
		using namespace boost::asio;

		boost::asio::detail::async_result_init<
			RealHandler, void(boost::system::error_code)> init(
			BOOST_ASIO_MOVE_CAST(RealHandler)(handler));

		std::pair<avif::auto_avPacketPtr, boost::function<void(boost::system::error_code)> > value(
			avPacket,
			init.handler
		);

		avinterface->_write_queue->push(value);

		return init.result.get();
	}

	void async_interface_write_packet(avif * avinterface, avif::auto_avPacketPtr avPacket)
	{
		std::pair<avif::auto_avPacketPtr, boost::function<void(boost::system::error_code)> > value(
			avPacket,
			[](boost::system::error_code){}
		);

		avinterface->_write_queue->push(value);
	}

	// 读取这个接口上的数据，然后转发数据！
	void interface_runner(avif avinterface, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;

		while (!(*avinterface.quitting))
		{
			// 读取一个数据包
			boost::shared_ptr<proto::avpacket> avpkt = avinterface.async_read_packet(yield_context[ec]);

			if (avpkt)
			{
				// 这里是数据包的前置处理
				auto it = m_async_wait_packet_pred_handler_preprocess_list.begin();
				while (it != m_async_wait_packet_pred_handler_preprocess_list.end())
				{
					if (it->pred(boost::ref(*avpkt)))
					{
						it->handler(boost::system::error_code());
						m_async_wait_packet_pred_handler_preprocess_list.erase(it++);
					}
					else
					{
						it++;
					}
				}

				boost::asio::spawn(io_service, boost::bind(&avkernel_impl::process_recived_packet, shared_from_this(), avpkt, avinterface, _1));
			}
			else
			{
				* avinterface.quitting = true;
			}
		}
		avinterface._write_queue->cancele();
		remove_interface(avinterface.get_ifname());
	}

	void async_sendto_op(std::string target, std::string data, avkernel::ReadyHandler handler, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;
		/*
		*
		* 我来说一下算法, 首先根据路由表选定需要的 interface
		*
		* 接着寻找的加密用的公钥，如果本地的缓存没找到，就要首先发送一个 askpk 消息获取公钥
		*
		* 获取到公钥后，调用 RSA_public_encrypt 加密数据，再用
		* RSA_private_encrypt 加密，构建出 avPacket.
		* 然后再调用 interface->async_write_packet 将数据发送出去
		*/

		avif* interface = select_route(target);
		if (!interface)
		{
			ec = boost::asio::error::network_unreachable;
			// 返回 no route to host 错误
			handler(ec);
			return ;
		}

		RSA* target_pubkey = find_RSA_pubkey(target);

		if (!target_pubkey)
		{
			for (int i = 0; !target_pubkey && i < 3; i++)
			{
				// 进入 askpk 模式
				// TODO 如果如果已经有一个了，则不用发送，直接等待
				interface = select_route(target);
				if (interface)
					async_send_agmp_pkask(interface, target);
				else
					return handler(boost::asio::error::network_unreachable);

				// TODO 发送 askpk 消息获取共钥
				// TODO 如果配置了公钥服务器，尝试去公钥服务器获取
				boost::posix_time::ptime deadline = boost::posix_time::microsec_clock::local_time();

				deadline += boost::posix_time::seconds(8);

				volatile bool no_route_to_host = false;

				async_wait_processed_packet(deadline, [this, target, &no_route_to_host](const proto::avpacket & pkt)->bool
				{
					if (av_address_to_string(pkt.src()) == target && find_RSA_pubkey(target))
						return true;

					if (pkt.upperlayerpotocol() == "agmp")
					{
						proto::agmp agmp;
						agmp.ParseFromString(pkt.payload());
						// 看是不是告诉自己的
						if (agmp.has_noroutetohost())
						{
							return no_route_to_host = (av_address_to_string(agmp.noroutetohost().host()) == target);
						}
					}

					return false;
				}, yield_context[ec]);

				if (no_route_to_host)
				{
					return handler(boost::asio::error::network_unreachable);
				}

				target_pubkey = find_RSA_pubkey(target);
			}

			if (!target_pubkey)
			{
				return handler(boost::asio::error::network_unreachable);
			}
		}

		// TODO 构造 avPacket

		proto::avpacket avpkt;

//		target_pubkey = interface->get_rsa_key();
		// 第一次加密
		std::string first_pubencode = RSA_public_encrypt(target_pubkey, data);
		// 第二次签名
		std::string second_sign = RSA_private_encrypt(interface->get_rsa_key(), first_pubencode);

		// 把加密后的数据写入avPacket
		avpkt.set_payload(second_sign);

		// 附上自己的 publickey
		std::string pubkey;
		pubkey.resize(BN_num_bytes(interface->get_rsa_key()->n));

		BN_bn2bin(interface->get_rsa_key()->n,(uint8_t*)&pubkey[0]);

		avpkt.set_publickey(pubkey);
		avpkt.mutable_dest()->CopyFrom(av_address_from_string(target));
		avpkt.mutable_src()->CopyFrom(*interface->if_address());
		avpkt.set_upperlayerpotocol("avim");

		// TODO 添加其他
		avpkt.set_time_to_live(64);

		// 添入发送列队
		avif::auto_avPacketPtr avpktptr;
		// 空 deleter
		avpktptr.reset(&avpkt, [](void*){});
		async_interface_write_packet(interface, avpktptr, yield_context[ec]);

		// FIXME 这里为何调用不到
		// 做完成通知
		handler(ec);
	}

	void async_sendto(const std::string & target, const std::string & data, avkernel::ReadyHandler handler)
	{
		boost::asio::spawn(io_service, boost::bind(&avkernel_impl::async_sendto_op, shared_from_this(), target, data, handler, _1 ));
	}

	int sendto(const std::string & target, const std::string & data)
	{
		boost::system::error_code ec;

		boost::mutex m;
		boost::unique_lock< boost::mutex > l(m);
		boost::condition_variable ready;

		async_sendto(target, data, [&ec, &ready](const boost::system::error_code & _ec)
			{ ec = _ec; ready.notify_all();});


		ready.wait(l);
		return ec.value();
	}

	// 从本地数据找 RSA 公钥，找不到就返回 NULL , 让 agmp 协议其作用
	RSA* find_RSA_pubkey(std::string address) const
	{
		if (trusted_pubkey.count(address) > 0)
		{
			return trusted_pubkey.at(address).keys.front().rsa.get();
		}
		return NULL;
	}

	void add_RSA_pubkey(std::string address, RSA* pubkey, boost::posix_time::ptime valid_until)
	{
		auto& key_item = trusted_pubkey[address];
		key_item.avaddr = address;
		AVPubKey key;
		RSA_up_ref(pubkey);
		key.rsa = boost::shared_ptr<RSA>(pubkey, RSA_free);
		key.valid_until = valid_until;
		key_item.keys.push_back(key);
		std::sort(key_item.keys.begin(), key_item.keys.end(), [](const AVPubKey& lhs, const AVPubKey& rhs){ return lhs.valid_until > rhs.valid_until; });
	}

	void purge_RSA_pubkey()
	{
		boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
		for (auto iter = trusted_pubkey.begin(); iter != trusted_pubkey.end(); ++iter)
		{
			auto& keys = iter->second.keys;
			for(int i = 0; i < keys.size(); ++i)
			{
				if(keys[i].valid_until < now)
				{
					keys.erase(keys.begin() + i, keys.end());
					break;
				}
			}
			if(keys.size() == 0)
			{
				trusted_pubkey.erase(iter++);
			}
		}
	}

	// 移除接口的时候调用
	void remove_interface(std::string avifname)
	{
		std::cerr << "interface " << avifname << " removed " << std::endl;
		// TODO 移除路由表上使用该接口的所有项目

		auto it = m_routes.begin();

		while ((it = std::find_if(m_routes.begin(), m_routes.end(), [avifname](const RouteItem & item){return item.ifname == avifname;})) != m_routes.end())
		{
			m_routes.erase(it);
		}
		m_avifs.erase(avifname);
	}

	avif* select_route(std::string address)
	{
		for (auto iter = m_routes.begin(); iter != m_routes.end(); ++iter)
		{
			if (boost::regex_match(address, iter->pattern))
			{
				return &m_avifs.at(iter->ifname);
			}
		}
		return nullptr;
	}

	bool add_route(std::string targetAddress, std::string gateway, std::string ifname, int metric)
	{
		/*
		* 将目标地址添加到路由表  targetAddress 是正则表达式的
		*/
		assert(m_avifs.count(ifname) > 0);
		if (m_avifs.count(ifname) > 0)
		{
			m_routes.push_back(RouteItem{boost::regex(targetAddress), gateway, ifname, metric});
			boost::sort(m_routes);
			return true;
		}
		else
		{
			return false;
		}
	}

	void async_send_agmp_pkask(avif* interface, const std::string& target)
	{
		avif::auto_avPacketPtr pkt(new proto::avpacket);

		*pkt->mutable_src() = *interface->if_address();
		*pkt->mutable_dest() = av_address_from_string(target);
		pkt->set_upperlayerpotocol("pkreply");
		pkt->set_upperlayerpotocol("pkask");
		pkt->set_time_to_live(64);

		async_interface_write_packet(interface, pkt);
	}

	void async_send_agmp_pkreply(avif* interface, const std::string& target)
	{
		avif::auto_avPacketPtr pkt(new proto::avpacket);

		*pkt->mutable_src() = *interface->if_address();
		*pkt->mutable_dest() = av_address_from_string(target);
		pkt->set_upperlayerpotocol("pkreply");
		pkt->set_time_to_live(64);

		unsigned char* out = NULL;
		int certlen = i2d_X509((X509*)interface->get_cert(), &out);
		pkt->set_payload(out, certlen);
		OPENSSL_free(out);

		async_interface_write_packet(interface, pkt);
	}

	template<class Pred, class CompleteHandler>
	void async_wait_processed_packet(const boost::posix_time::ptime& deadline, Pred pred, CompleteHandler handler)
	{
		using namespace boost::asio;

		boost::asio::detail::async_result_init<
			CompleteHandler, void(boost::system::error_code)> init(
			BOOST_ASIO_MOVE_CAST(CompleteHandler)(handler));


		async_wait_packet_pred_handler item;
		item.deadline = deadline;
		item.pred = pred;
		item.handler = init.handler;

		m_async_wait_packet_pred_handler_postprocess_list.push_back(std::move(item));

		init.result.get();
	}

	void timer1_start()
	{
		if (m_quitting)
			return;
		timer1.expires_from_now(boost::posix_time::milliseconds(500));
		timer1.async_wait(boost::bind(&avkernel_impl::timer1_tick, shared_from_this(),_1));
	}

	void timer2_start()
	{
		if (m_quitting)
			return;
		timer2.expires_from_now(boost::posix_time::seconds(300));
		timer2.async_wait(boost::bind(&avkernel_impl::timer2_tick, shared_from_this(),_1));
	}
	void timer2_tick(boost::system::error_code ec)
	{
		timer2_start();
		purge_RSA_pubkey();
	}

	void timer1_tick(boost::system::error_code ec)
	{
		timer1_start();

		auto now = boost::posix_time::microsec_clock::local_time();

		auto it = m_async_wait_packet_pred_handler_postprocess_list.begin();

		// 检查有无过期的
		while (it != m_async_wait_packet_pred_handler_postprocess_list.end())
		{
			if (it->deadline < now)
			{
				it->handler(boost::asio::error::timed_out);
				m_async_wait_packet_pred_handler_postprocess_list.erase(it++);
			}
			else
			{
				it ++;
			}
		}

		it = m_async_wait_packet_pred_handler_preprocess_list.begin();

		while (it != m_async_wait_packet_pred_handler_preprocess_list.end())
		{
			if (it->deadline < now)
			{
				it->handler(boost::asio::error::timed_out);
				m_async_wait_packet_pred_handler_preprocess_list.erase(it++);
			}
			else
			{
				it ++;
			}
		}
	}

public:
	avkernel_impl(boost::asio::io_service& _io_service)
		: io_service(_io_service)
		, m_recv_buffer(io_service)
		, m_root_ca_cert([](){
			boost::shared_ptr<BIO> bp(BIO_new_mem_buf((void*)avim_root_ca_certificate_string, strlen(avim_root_ca_certificate_string)), BIO_free);
			return PEM_read_bio_X509(bp.get(), 0, 0, 0);
		}())
		, timer1(io_service)
		, timer2(io_service)
	{
		m_quitting = false;
	}

	~avkernel_impl()
	{
		X509_free((X509*)m_root_ca_cert);
	}

	std::atomic<bool> m_quitting;
	boost::asio::deadline_timer timer1, timer2;
	friend avkernel;
};

}

avkernel::avkernel(boost::asio::io_service& _io_service)
	: io_service(_io_service)
{
	_impl = boost::make_shared<detail::avkernel_impl>(boost::ref(io_service));
	_impl->timer1_start();
	_impl->timer2_start();
}

avkernel::~avkernel()
{
	_impl->m_quitting = true;
}

bool avkernel::add_interface(avif avinterface)
{
	std::cout << "new interface " << avinterface.get_ifname() << " created. remotepoint : "
		<< av_address_to_string(*avinterface.remote_address()) << std::endl;

	_impl->m_avifs.insert(std::make_pair(avinterface.get_ifname(), avinterface));
	boost::asio::spawn(io_service, boost::bind(&detail::avkernel_impl::interface_runner, _impl, avinterface, _1));
	boost::asio::spawn(io_service, boost::bind(&detail::avkernel_impl::interface_writer, _impl, avinterface, _1));
	return _impl->m_avifs.find(avinterface.get_ifname()) != _impl->m_avifs.end();
}

bool avkernel::add_route(std::string targetAddress, std::string gateway, std::string ifname, int metric)
{
	return _impl->add_route(targetAddress, gateway, ifname, metric);
}

int avkernel::sendto(const std::string& target, const std::string& data)
{
	return _impl->sendto(target, data);
}

int avkernel::recvfrom(std::string& target, std::string& data)
{
	return _impl->recvfrom(target, data);
}

void avkernel::async_sendto(const std::string & target, const std::string & data, ReadyHandler handler)
{
	_impl->async_sendto(target, data, handler);
}

void avkernel::async_sendto(const std::string& target, const std::string& data, boost::asio::yield_context yield_context)
{
	using namespace boost::asio;

	boost::asio::detail::async_result_init<
		boost::asio::yield_context, void(boost::system::error_code)> init((boost::asio::yield_context&&)yield_context);

	async_sendto(target, data, init.handler);

	return init.result.get();
}

void avkernel::async_recvfrom(std::string& target, std::string& data, ReadyHandler handler)
{
	_impl->async_recvfrom(target, data, handler);
}

void avkernel::async_recvfrom(std::string& target, std::string& data, boost::asio::yield_context yield_context)
{
	using namespace boost::asio;

	boost::asio::detail::async_result_init<
		boost::asio::yield_context, void(boost::system::error_code)> init((boost::asio::yield_context&&)yield_context);

	async_recvfrom(target, data, init.handler);

	return init.result.get();
}

const X509* avkernel::get_root_ca()
{
	return (const X509*)_impl->m_root_ca_cert;
}
