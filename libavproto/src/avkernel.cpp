
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/regex.hpp>
#include <boost/range/algorithm.hpp>

#include <openssl/x509.h>
#include <openssl/pem.h>

#include "avif.hpp"
#include "avproto.hpp"
#include "async_coro_queue.hpp"
#include "protocol/avim-base.pb.h"

extern const char * avim_root_ca_certificate_string;

template<typename C, typename Pred>
auto container_remove_all(C & c, Pred pred) -> decltype(std::begin(c))
{
	auto it = std::begin(c);
	while( (it = std::find_if(it, std::end(c), pred) ) != std::end(c))
	{
		c.erase(it++);
	}
	return it;
}

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

	struct AVPubKey : boost::noncopyable {
		autoRSAptr rsa;
		// valid_until this time
		std::time_t valid_until;
	};

	struct AVdbitem{
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

	// 缓存接收到的包以便后续处理的那种
	std::list<avif::auto_avPacketPtr> m_recv_cache;

	bool is_to_me(const proto::base::avAddress & addr)
	{
		// 遍历 interface 做比较
		for( const auto & i : m_avifs)
		{
			if(addr.username() == i.second.if_address()->username() &&
				addr.domain() == i.second.if_address()->domain()
			){
				return true;
			}
		}
		return false;
	}

	// TODO 数据包接收过程的完整实现， 目前只实现个基础的垃圾
	void process_recived_packet_to_me(boost::shared_ptr<proto::base::avPacket> avPacket, avif avinterface, boost::asio::yield_context yield_context)
	{
		std::cerr << "one pkt from " <<  av_address_to_string(avPacket->src()) << " sended to me" << std::endl;
		std::string add, data;

		// TODO 处理 agmp 协议等等等等
		if( ! avPacket->has_payload() )
			return ;

		add = av_address_to_string(avPacket->src());

		avPacket->publickey();

		autoRSAptr rsa(RSA_new(), RSA_free);

		rsa->e = BN_new();
		BN_set_word(rsa->e, 65537);
		rsa->n = BN_bin2bn((const unsigned char*) avPacket->publickey().data(), avPacket->publickey().length(), rsa->n);

		// 第一阶段解密，先使用发送者的公钥解密
		std::string stage1decypted;
		stage1decypted.resize(RSA_size(rsa.get()) + avPacket->payload().length());
		// data.res
		stage1decypted.resize(
			RSA_public_decrypt(
				avPacket->payload().length(),
				(uint8_t*) avPacket->payload().data(),
				(uint8_t*) &stage1decypted[0],
				rsa.get(),
				RSA_PKCS1_PADDING
			)
		);

		// 第二阶段解密，用自己的私钥解密
		// 因为 find_RSA_pubkey 还没实现，所以发送方没有加密
		// 暂时跳过
#if 0
		data.resize( RSA_size(avinterface.get_rsa_key()) + stage1decypted.length() );

		data.resize(
			RSA_private_decrypt(
				stage1decypted.length(),
				(uint8_t*) stage1decypted.data(),
				(uint8_t*) &data[0],
				avinterface.get_rsa_key(),
				RSA_PKCS1_OAEP_PADDING
			)
		);
#endif
		data = stage1decypted;

		// 挂入本地接收列队，等待上层读取
		m_recv_buffer.push(std::make_pair(add, data));
	}


	void process_recived_packet(boost::shared_ptr<proto::base::avPacket> avPacket, avif avinterface, boost::asio::yield_context yield_context)
	{

		RSA * stored_key = find_RSA_pubkey(av_address_to_string(avPacket->src()));

		// TODO 执行客户端校验
		if(avPacket->has_publickey() && stored_key)
		{
			// 有 pubkey ， 比较内部数据库里的记录
			// 如果和内部数据库的记录不一致
			// 要使用 avgmp 协议请求一份证书来校验一遍

			// 干这种事情的话，就需要开另外一个协程慢慢干了，呵呵
			// 因此加到 TODO 列表

		}
		else if(!stored_key && ! avPacket->has_publickey())
		{
			// TODO, 执行 证书请求，并在请求通过后，验证数据包接受该
		}else
		{
			 // TODO 执行 证书请求，并在请求通过后，验证数据包接受该
		}

		// 查看据地地址，如果是自己，就交给上层

		if( is_to_me(avPacket->dest()) )
		{
			return process_recived_packet_to_me(avPacket, avinterface, yield_context);
		}

		// 查看 ttl
		if( avPacket->time_to_live() <= 1)
		{
			// 丢弃包
			// TODO 向原地址返回一个 ttl 消尽的agmp消息
			return ;
		}

		avPacket->set_time_to_live(avPacket->time_to_live() - 1);

		std::cerr << "got one pkt from " <<  av_address_to_string(avPacket->src())
			<< " to " << av_address_to_string(avPacket->dest());

		// TODO 查找路由表
		avif * interface = select_route(av_address_to_string(avPacket->dest()));

		if(!interface)
		{
			// TODO 返回 no route to host 消息
			std::cerr << "|| but no route to host, dropping packet!!" << std::endl;
			return;
		}

		std::cerr << " , now routing with " << interface->get_ifname() << std::endl;

		// 转发过去
		async_interface_write_packet(interface, avPacket, yield_context);

		std::cerr << "routed ! " << std::endl;

	}

	void async_recvfrom_op(std::string & target, std::string & data, avkernel::ReadyHandler handler, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;

		auto _data_pair = m_recv_buffer.async_pop(yield_context[ec]);
		if(ec)
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

		for(; ! (* avinterface.quitting) ;)
		{
			std::pair<avif::auto_avPacketPtr, boost::function<void(boost::system::error_code)>>
				popvalue = avinterface._write_queue->async_pop(yield_context[ec]);
			if(ec)
			{
				break;
			}
			avinterface.async_write_packet(popvalue.first.get(), yield_context[ec]);
			popvalue.second(ec);
		}
		std::cerr << "interface_writer coroutine exited" << std::endl;
	}

	template<class RealHandler>
	inline BOOST_ASIO_INITFN_RESULT_TYPE(RealHandler,
		void(boost::system::error_code))
	async_interface_write_packet(avif * avinterface, avif::auto_avPacketPtr avPacket, BOOST_ASIO_MOVE_ARG(RealHandler) handler)
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

	// 读取这个接口上的数据，然后转发数据！
	void interface_runner(avif avinterface, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;

		for(; ! (* avinterface.quitting) ;)
		{
			// 读取一个数据包
			boost::shared_ptr<proto::base::avPacket> avpkt = avinterface.async_read_packet(yield_context[ec]);

			if(avpkt)
			{
				// boost::asio::spawn(io_service, boost::bind(&avkernel_impl::process_recived_packet, shared_from_this(), avpkt, avinterface, _1));
				process_recived_packet(avpkt, avinterface, yield_context);
			}else
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

		avif * interface = select_route(target);
		if( !interface )
		{
			ec = boost::asio::error::network_unreachable;
			// 返回 no route to host 错误
			handler(ec);
			return ;
		}

		RSA * target_pubkey = find_RSA_pubkey(target);

		if( ! target_pubkey )
		{
			// 进入 askpk 模式
			// TODO 如果如果已经有一个了，则不用发送，直接等待
			async_send_agmp_pkask(interface, target, yield_context);

			// TODO 发送 askpk 消息获取共钥
			// TODO 如果配置了公钥服务器，尝试去公钥服务器获取
		}

		// TODO 构造 avPacket

		proto::base::avPacket avpkt;

		// 第一次加密
		std::string first_pubencode;

		int encrypted_size;
		// target_pubkey 没实现，为了测试暂时不加密
//		first_pubencode.resize(RSA_size(target_pubkey) + data.length());
//		first_pubencode.resize(RSA_public_encrypt(data.length(), (uint8_t*) data.data(),(uint8_t*) &first_pubencode[0], target_pubkey, RSA_PKCS1_OAEP_PADDING));

		first_pubencode = data;

		// 第二次签名
		std::string second_sign;
		second_sign.resize(RSA_size(interface->get_rsa_key()) + data.length());
		encrypted_size = RSA_private_encrypt(
				first_pubencode.length(),
				(uint8_t*) first_pubencode.data(),
				(uint8_t*) &second_sign[0],
				interface->get_rsa_key(),
				RSA_PKCS1_PADDING
			);

		second_sign.resize(encrypted_size);

		// 把加密后的数据写入avPacket
		avpkt.set_payload(second_sign);

		// 附上自己的 publickey
		std::string pubkey;
		pubkey.resize(BN_num_bytes(interface->get_rsa_key()->n));

		BN_bn2bin(interface->get_rsa_key()->n,(uint8_t*) &pubkey[0]);

		avpkt.set_publickey(pubkey);
		avpkt.mutable_dest()->CopyFrom( av_address_from_string(target) );
		avpkt.mutable_src()->CopyFrom( *interface->if_address() );
		avpkt.set_upperlayerpotocol("avim");

		// TODO 添加其他
		avpkt.set_time_to_live(64);

		// 添入发送列队
		avif::auto_avPacketPtr avpktptr;
		// 空 deleter
		avpktptr.reset( & avpkt, [](void*){});
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
	RSA * find_RSA_pubkey(std::string address)
	{
		return NULL;
	}

	// 移除接口的时候调用
	void remove_interface(std::string avifname)
	{
		std::cerr << "interface " << avifname << " removed " << std::endl;
		// TODO 移除路由表上使用该接口的所有项目

		auto it = m_routes.begin();

		while( (it = std::find_if(m_routes.begin(), m_routes.end(), [avifname](const RouteItem & item){return item.ifname == avifname;}) ) != m_routes.end())
		{
			m_routes.erase(it);
		}
		m_avifs.erase(avifname);
	}

	avif * select_route(std::string address)
	{
		for (auto iter = m_routes.begin(); iter != m_routes.end(); ++iter)
		{
			if(boost::regex_match(address, iter->pattern))
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
		if(m_avifs.count(ifname) > 0)
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

	void async_send_agmp_pkask(avif * interface, const std::string & target, boost::asio::yield_context yield_context)
	{
 		proto::base::avPacket pkt;
		* pkt.mutable_src() = * interface->if_address();
		* pkt.mutable_dest() = av_address_from_string(target);
		pkt.set_upperlayerpotocol("pkask");

	}

public:
	avkernel_impl(boost::asio::io_service & _io_service)
		: io_service(_io_service)
		, m_recv_buffer(io_service)
		, m_root_ca_cert([](){
			boost::shared_ptr<BIO> bp(BIO_new_mem_buf((void*)avim_root_ca_certificate_string, strlen(avim_root_ca_certificate_string)), BIO_free);
			return PEM_read_bio_X509(bp.get(), 0, 0, 0);
		}())
	{

	}

	~avkernel_impl()
	{
		X509_free((X509*)m_root_ca_cert);
	}

	friend avkernel;
};

}

avkernel::avkernel(boost::asio::io_service & _io_service)
	: io_service(_io_service)
{
	_impl = boost::make_shared<detail::avkernel_impl>(boost::ref(io_service));
}

bool avkernel::add_interface(avif avinterface)
{
	std::cout << "new interface " << avinterface.get_ifname() << " created. remotepoint : "
		<< av_address_to_string(*avinterface.remote_address()) << std::endl;

	_impl->m_avifs.insert(std::make_pair(avinterface.get_ifname(), avinterface));
	boost::asio::spawn(io_service, boost::bind(&detail::avkernel_impl::interface_runner, _impl, avinterface, _1));
	boost::asio::spawn(io_service, boost::bind(&detail::avkernel_impl::interface_writer, _impl, avinterface, _1));
	return true;
}

bool avkernel::add_route(std::string targetAddress, std::string gateway, std::string ifname, int metric)
{
	return _impl->add_route(targetAddress, gateway, ifname, metric);
}

int avkernel::sendto(const std::string & target, const std::string & data)
{
	return _impl->sendto(target, data);
}

int avkernel::recvfrom(std::string & target, std::string &data)
{
	return _impl->recvfrom(target, data);
}

void avkernel::async_sendto(const std::string & target, const std::string & data, ReadyHandler handler)
{
	_impl->async_sendto(target, data, handler);
}

void avkernel::async_sendto(const std::string & target, const std::string & data, boost::asio::yield_context yield_context)
{
	using namespace boost::asio;

	boost::asio::detail::async_result_init<
		boost::asio::yield_context, void(boost::system::error_code)> init((boost::asio::yield_context&&)yield_context);

	async_sendto(target, data, init.handler);

	return init.result.get();
}

void avkernel::async_recvfrom(std::string & target, std::string & data, ReadyHandler handler)
{
	_impl->async_recvfrom(target, data, handler);
}

void avkernel::async_recvfrom(std::string & target, std::string & data, boost::asio::yield_context yield_context)
{
	using namespace boost::asio;

	boost::asio::detail::async_result_init<
		boost::asio::yield_context, void(boost::system::error_code)> init((boost::asio::yield_context&&)yield_context);

	async_recvfrom(target, data, init.handler);

	return init.result.get();
}

const X509 * avkernel::get_root_ca()
{
	return (const X509 *)_impl->m_root_ca_cert;
}
