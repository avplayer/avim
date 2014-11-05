
#pragma once

#include <boost/noncopyable.hpp>

#include "avif.hpp"

// 这个是 TCP 协议的接口
struct avtcpif : boost::noncopyable
{
public:
	avtcpif(boost::shared_ptr<boost::asio::ip::tcp::socket>, std::string local_addr, RSA * _key, X509 *);
	~avtcpif();

	// TCP接口，有 master/slave 模式之分 服务器使用 master 模式，客户端则是 slave 模式
	// 区别只是登录时候的握手不一样，所以就在 handshake 这里直接设定就可以了
	bool async_master_handshake(bool as_master, boost::asio::yield_context yield_context);

	// 同步模式
	bool slave_handshake(bool as_master);

	std::string remote_addr();

public: // 下面是实现 avif 接口
	boost::asio::io_service & get_io_service() const;
	std::string get_ifname() const;
	const proto::base::avAddress * if_address() const;
	const proto::base::avAddress * remote_address() const;
	RSA * get_rsa_key();
	boost::shared_ptr<proto::base::avPacket> async_read_packet(boost::asio::yield_context yield_context);
    bool async_write_packet(proto::base::avPacket*, boost::asio::yield_context yield_context);

private:
	// 分配一个 if 接口名字
	static std::string allocate_ifname();

	std::string ifname;
	RSA * _rsa;
	X509 * _x509;
	boost::scoped_ptr<proto::base::avAddress> m_local_addr;
	boost::scoped_ptr<proto::base::avAddress> m_remote_addr;
	boost::shared_ptr<boost::asio::ip::tcp::socket> m_sock;

	boost::asio::streambuf m_recv_buf, m_send_buf;
};

