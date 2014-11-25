#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <openssl/rsa.h>
#include <openssl/x509.h>

#include "avim_proto/address.pb.h"
#include "avim_proto/im.pb.h"

#include "avproto.hpp"

class avjackif;
// 呐, 这个才是真正真正的 avim 协议实现. 这类主要处理 IM 层协议
class avim_client
{
public:
	typedef boost::function<void(boost::system::error_code, proto::av_address, proto::avim_message_packet)> RecvHandlerType;
	typedef boost::function<void(boost::system::error_code)> SendHandlerType;
	typedef boost::function<bool(proto::av_address)> SelectDecryptKeyCallbackType;
public:
	// 使用密钥和证书构建 avim 对象
	// io_service.run() 执行起来后, 这个就开始自动登录了, 登录自动重
	avim_client(boost::asio::io_service &, std::string privatekey, std::string public_cert);

	// 异步消息接收
	void async_recv_im(SelectDecryptKeyCallbackType, RecvHandlerType);
	void async_recv_im(SelectDecryptKeyCallbackType, proto::av_address&, proto::avim_message_packet&, boost::asio::yield_context yield_context);

	void async_send_im(const proto::av_address&, const proto::avim_message_packet& pkt, avim_client::SendHandlerType);
	void async_send_im(const proto::av_address&, const proto::avim_message_packet& pkt, boost::asio::yield_context yield_context);
	void async_send_im(const proto::av_address&, const proto::avim_message_packet&, std::string encrypt_key , SendHandlerType);
	void async_send_im(const proto::av_address&, const proto::avim_message_packet&, int flag, SendHandlerType);

	// 等待在线, 如果已经在线会立即返回
	void async_wait_online(boost::asio::yield_context yield_context);
	void async_wait_online(boost::function<void()> handler);

	proto::av_address self_address();
private:

	void coroutine_login(boost::asio::yield_context yield_context);

private:
	boost::asio::io_service & io_service;
	avkernel m_avkernel;

	std::atomic<bool> m_online;

	std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
	std::shared_ptr<avjackif> m_avinterface;

	std::shared_ptr<RSA> m_rsa_key;
	std::shared_ptr<X509> m_x509_cert;
};

