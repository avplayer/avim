
#include <openssl/dh.h>

#include "serialization.hpp"

#include "avjackif.hpp"

#include "avim_proto/message.pb.h"
#include "avim_proto/user.pb.h"

/*
 * 这个类呢，是用来实现和 JACK 实现的那个 AVROUTER 对接的。也就是 JACK 版本的 AV NETWORK SERVICE PROVIDER
 */

// av地址可以从证书里获取，所以外面无需传递进来
avjackif::avjackif(boost::asio::io_service & _io_service)
	: m_sock(_io_service)
{

}

void avjackif::async_handshake(std::string login_username, std::string login_password, boost::asio::yield_context yield_context)
{
	proto::client_hello client_hello;
	client_hello.set_client("avim");
	client_hello.set_version(0001);

	unsigned char to[100];

	auto dh = DH_new();
	DH_generate_parameters_ex(dh,64,DH_GENERATOR_5,NULL);

	// 把 5 传过去
	client_hello.set_random_key((const void*)to, BN_bn2bin(dh->g, to));

	auto tobesend = av_router::encode(client_hello);

	boost::asio::async_write(m_sock, boost::asio::buffer(tobesend), yield_context);

	std::uint32_t l;
	boost::asio::async_read(m_sock, boost::asio::buffer(&l, sizeof(l)), yield_context);

	std::string  buf;
	buf.resize(htonl(l));
	boost::asio::async_read(m_sock, boost::asio::buffer(&buf[0], htonl(l)), yield_context);

	// 解码
	proto::server_hello * server_hello = (proto::server_hello *) av_router::decode(buf);

	dh->p =BN_bin2bn((const unsigned char *) server_hello->random_key().data(), server_hello->random_key().length(), dh->p);
	DH_generate_key(dh);

	m_shared_key.resize(DH_size(dh));
	// 密钥就算出来啦！
	DH_compute_key(&m_shared_key[0], dh->pub_key, dh);
	DH_free(dh);
}
