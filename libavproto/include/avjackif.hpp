

#pragma once

#include <boost/noncopyable.hpp>

#include "avif.hpp"

// 这个是 和 JACK 写的 router 对接的接口
struct avjackif : boost::noncopyable
{
	std::vector<unsigned char> m_shared_key;
public:
	avjackif(boost::asio::io_service & _io_service);
	~avjackif();

	void set_pki(RSA * _key, X509 *);

	// TCP接口，有 master/slave 模式之分 服务器使用 master 模式，客户端则是 slave 模式
	// 区别只是登录时候的握手不一样，所以就在 handshake 这里直接设定就可以了
	void async_handshake(std::string login_username, std::string login_password, boost::asio::yield_context yield_context);


	void async_register_new_user(std::string user_name);

	std::string remote_addr();

public: // 下面是实现 avif 接口
	boost::asio::io_service & get_io_service() const;
	std::string get_ifname() const;
	const proto::avAddress * if_address() const;
	const proto::avAddress * remote_address() const;
	RSA * get_rsa_key();
	X509 * get_cert();

	void set_root_ca(X509 * ca);
	boost::shared_ptr<proto::avPacket> async_read_packet(boost::asio::yield_context yield_context);
    bool async_write_packet(proto::avPacket*, boost::asio::yield_context yield_context);

protected:
	bool check_cert(const std::string & cert);

private:
	// 分配一个 if 接口名字
	static std::string allocate_ifname();

	std::string ifname;
	RSA * _rsa;
	X509 * _x509, *m_root_ca;
	boost::scoped_ptr<proto::avAddress> m_local_addr;
	boost::scoped_ptr<proto::avAddress> m_remote_addr;

	boost::asio::ip::tcp::socket m_sock;
	boost::asio::streambuf m_recv_buf, m_send_buf;
};

