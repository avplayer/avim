#pragma once

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <memory>
#include "avim_proto/address.pb.h"
#include "avim_proto/im.pb.h"

// 呐, 这个才是真正真正的 avim 协议实现. 这类主要处理 IM 层协议
class avim
{
public:
	boost::function<void(boost::system::error_code, proto::av_address, proto::avim_packet)> RecvHandlerType;
	boost::function<void(boost::system::error_code)> SendHandlerType;
public:
	// 使用密钥和证书构建 avim 对象
	avim(boost::asio::io_service &, std::string privatekey, std::string public_cert);


	// 异步消息接收
	void async_recv_im(RecvHandlerType);
	void async_send_im(proto::av_address, proto::avim_packet, SendHandlerType);
	void async_send_im(proto::av_address, proto::avim_packet, int flag, SendHandlerType);
private:
	boost::asio::io_service & io_service;
};

