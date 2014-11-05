
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include <avtcpif.hpp>
#include <avproto.hpp>

#include <openssl/x509.h>
#include <openssl/pem.h>

// 一个非常非常简单的 IM 实现，测试用途

static boost::asio::io_service io_service;

avkernel avcore(io_service);

static void msg_reader(boost::asio::yield_context yield_context)
{
	boost::system::error_code ec;
	std::string sender, data;

	for(;;)
	{
		avcore.async_recvfrom(sender, data, yield_context);

		std::cerr << "接收到的数据： " << data << std::endl;
	}
}

int main(int argv, char * argc[])
{
	boost::shared_ptr<BIO> keyfile(BIO_new_file("avim.key", "r"), BIO_free);
	if(!keyfile)
	{
		std::cerr << "can not open avim.key" << std::endl;
		exit(1);
	}
	RSA * rsa_key = PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, 0, 0);

	boost::shared_ptr<BIO> certfile(BIO_new_file("avim.crt", "r"), BIO_free);
	if(!certfile)
	{
		std::cerr << "can not open avim.crt" << std::endl;
		exit(1);
	}
	X509 * x509_cert = PEM_read_bio_X509(certfile.get(), 0, 0, 0);

	certfile.reset();
	keyfile.reset();

	// 连接到 im.avplayer.org:24950

	boost::asio::ip::tcp::resolver resolver(io_service);

	boost::shared_ptr<boost::asio::ip::tcp::socket> avserver( new boost::asio::ip::tcp::socket(io_service));

	// 连接到 im.avplayer.org:24950
	boost::asio::connect(*avserver, resolver.resolve(boost::asio::ip::tcp::resolver::query("im.avplayer.org", "24950")));

	std::string me_addr = "test@avplayer.org";

	// 构造 avtcpif
	// 创建一个 tcp 的 avif 设备，然后添加进去
	boost::shared_ptr<avtcpif> avinterface;

	avinterface.reset(new avtcpif(avserver, me_addr, rsa_key, x509_cert) );

	avinterface->slave_handshake(0);

	avcore.add_interface(avinterface);

	// 添加路由表, metric越大，优先级越低
	avcore.add_route(".+@.+", me_addr, avinterface->get_ifname(), 100);

	// 进入 IM 过程，发送一个 test  到 test2@avplayer.org
	boost::async(
		[](){
			avcore.sendto("test@avplayer.org", "test, me are testing you stupid avim");
		}
	);

	// 开协程异步接收消息

	boost::asio::spawn(io_service, msg_reader);
	io_service.run();
}
