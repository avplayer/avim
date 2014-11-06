
#pragma once

#include <string>
#include <boost/scoped_ptr.hpp>

#include "avif.hpp"

namespace detail {
class avkernel_impl;
}

enum av_route_op{
	AVROUTE_ADD,
	AVROUTE_MOD,
	AVROUTE_DEL
};

// 从 private 可以里 dump 出 public key
static inline RSA * RSA_DumpPublicKey(RSA * pkey)
{
	RSA * pubkey = RSA_new();

	pubkey->e = BN_dup(pkey->e);
	pubkey->n = BN_dup(pkey->n);

	return pubkey;
}

class avkernel : boost::noncopyable
{
	boost::asio::io_service & io_service;
	boost::shared_ptr<detail::avkernel_impl> _impl;

	// ifname -> avif 的映射关系
public:

	typedef boost::function<void(boost::system::error_code)> ReadyHandler;

	avkernel(boost::asio::io_service &);
	~avkernel();

	bool add_interface(avif interface);

	// 添加一项路由
	bool add_route(std::string targetAddress, std::string gateway, std::string ifname, int metric);

	int sendto(const std::string & target, const std::string & data);
	int recvfrom(std::string & target, std::string &data);

	// 两个重载的异步发送，分别用于协程和回调
	// 因为不作为 header only 实现，故而不想在这里使用模板，所以只能重载了
	void async_sendto(const std::string & target, const std::string & data, ReadyHandler handler);
	void async_sendto(const std::string & target, const std::string & data, boost::asio::yield_context);

	// 两个重载的异步接收，分别用于协程和回调
	// 因为不作为 header only 实现，故而不想在这里使用模板，所以只能重载了
	void async_recvfrom(std::string & target, std::string & data, boost::asio::yield_context yield_context);
	void async_recvfrom(std::string & target, std::string & data, ReadyHandler handler);
    const X509 * get_root_ca();
};
