
#pragma once

#include <string>
#include <boost/scoped_ptr.hpp>
#include <algorithm>

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

/*
 * 顾名思义，这个是简单 RSA , c++ 封装，专门对付 openssl 烂接口烂源码烂文档这种弱智库的
 */

inline std::string RSA_public_encrypt(RSA * rsa, const std::string & from)
{
	std::string result;
	const int keysize = RSA_size(rsa);
	std::vector<unsigned char> block(keysize);
	const int chunksize = keysize  - RSA_PKCS1_PADDING_SIZE;
	int inputlen = from.length();

	for(int i = 0 ; i < inputlen; i+= chunksize)
	{
		auto resultsize = RSA_public_encrypt(std::min(chunksize, inputlen - i), (uint8_t*) &from[i],  &block[0], (RSA*) rsa, RSA_PKCS1_PADDING);
		result.append((char*)block.data(), resultsize);
	}
	return result;
}

inline std::string RSA_private_decrypt(RSA * rsa, const std::string & from)
{
	std::string result;
	const int keysize = RSA_size(rsa);
	std::vector<unsigned char> block(keysize);

	for(int i = 0 ; i < from.length(); i+= keysize)
	{
		auto resultsize = RSA_private_decrypt(std::min<int>(keysize, from.length() - i), (uint8_t*) &from[i],  &block[0], rsa, RSA_PKCS1_PADDING);
		result.append((char*)block.data(), resultsize);
	}
	return result;
}

inline std::string RSA_private_encrypt(RSA * rsa, const std::string & from)
{
	std::string result;
	const int keysize = RSA_size(rsa);
	std::vector<unsigned char> block(keysize);
	const int chunksize = keysize  - RSA_PKCS1_PADDING_SIZE;
	int inputlen = from.length();

	for(int i = 0 ; i < from.length(); i+= chunksize)
	{
		int flen = std::min<int>(chunksize, inputlen - i);

		std::fill(block.begin(),block.end(), 0);

		auto resultsize = RSA_private_encrypt(
			flen,
			(uint8_t*) &from[i],
			&block[0],
			rsa,
			RSA_PKCS1_PADDING
		);
		result.append((char*)block.data(), resultsize);
	}
	return result;
}

inline std::string RSA_public_decrypt(RSA * rsa, const std::string & from)
{
	std::string result;
	const int keysize = RSA_size(rsa);
	std::vector<unsigned char> block(keysize);

	int inputlen = from.length();

	for(int i = 0 ; i < from.length(); i+= keysize)
	{
		int flen = std::min(keysize, inputlen - i);

		auto resultsize = RSA_public_decrypt(
			flen,
			(uint8_t*) &from[i],
			&block[0],
			rsa,
			RSA_PKCS1_PADDING
		);
		result.append((char*)block.data(), resultsize);
	}
	return result;
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

	bool add_interface(avif avinterface);

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
