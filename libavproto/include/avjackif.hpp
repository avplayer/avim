

#pragma once

#include <boost/noncopyable.hpp>

#include "avif.hpp"

// 这个是 和 JACK 写的 router 对接的接口
struct avjackif : boost::noncopyable
{
	std::vector<unsigned char> m_shared_key;
public:
	avjackif(boost::shared_ptr<boost::asio::ip::tcp::socket> _sock);
	~avjackif();

	void set_pki(boost::shared_ptr<RSA> _key, boost::shared_ptr<X509>);

	// TCP接口，有 master/slave 模式之分 服务器使用 master 模式，客户端则是 slave 模式
	// 区别只是登录时候的握手不一样，所以就在 handshake 这里直接设定就可以了
	bool async_handshake(boost::asio::yield_context yield_context);
	// TCP接口，有 master/slave 模式之分 服务器使用 master 模式，客户端则是 slave 模式
	// 区别只是登录时候的握手不一样，所以就在 handshake 这里直接设定就可以了
	bool handshake();

	void async_register_new_user(std::string user_name);

	std::string remote_addr();

public: // 下面是实现 avif 接口
	boost::asio::io_service & get_io_service() const;
	std::string get_ifname() const;
	const proto::av_address * if_address() const;
	const proto::av_address * remote_address() const;
	RSA * get_rsa_key();
	X509 * get_cert();

	void set_root_ca(X509 * ca) { m_root_ca = ca;}
	boost::shared_ptr<proto::avpacket> async_read_packet(boost::asio::yield_context yield_context);
    bool async_write_packet(proto::avpacket*, boost::asio::yield_context yield_context);

protected:
	bool check_cert(const std::string & cert);

private:
	// 分配一个 if 接口名字
	static std::string allocate_ifname();

	std::string m_ifname;
	boost::shared_ptr<RSA> _rsa;
	boost::shared_ptr<X509> _x509;
	X509 *m_root_ca;

	boost::scoped_ptr<proto::av_address> m_local_addr;
	boost::scoped_ptr<proto::av_address> m_remote_addr;

	boost::shared_ptr<boost::asio::ip::tcp::socket> m_sock;
	boost::asio::streambuf m_recv_buf, m_send_buf;
};

