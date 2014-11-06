
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/make_shared.hpp>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

#include <avtcpif.hpp>
#include <avproto.hpp>

static boost::asio::io_service io_service;

avkernel avcore(io_service);

// 路由器接受用户地址后，每次都生成一个新的随机地址的
static std::string  get_new_radom_address()
{
	static int t = 0;

	return boost::str( boost::format("avrouter@avplayer.org/%d") % t ++ );
}

// 处理客户端
static void process_client(boost::asio::yield_context yielder, boost::shared_ptr<boost::asio::ip::tcp::socket> client_sock, RSA * rsa_key, X509 * x509_cert)
{
	boost::system::error_code ec;
	// 生成一个新的 av地址
	std::string me_addr = get_new_radom_address();

	// 创建一个 tcp 的 avif 设备，然后添加进去
	boost::shared_ptr<avtcpif> avinterface;

	avinterface.reset(new avtcpif(client_sock, me_addr, rsa_key, x509_cert) );
	avinterface->set_root_ca((X509*)avcore.get_root_ca());

	if(	avinterface->async_master_handshake(1, yielder[ec]) )
	{
		avcore.add_interface(avinterface);
		// 添加路由表
		avcore.add_route(avinterface->remote_addr(), me_addr, avinterface->get_ifname(), 0);
	}
}

static void async_acceptor(boost::asio::yield_context yielder, int port, RSA * privatekey, X509 * x509_cert)
{
	// 创建 tcp 端口，进入监听
	boost::asio::ip::tcp::acceptor listen_sock(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), port));

	while(true)
	{
		boost::system::error_code ec;
		boost::shared_ptr<boost::asio::ip::tcp::socket> client_sock(new boost::asio::ip::tcp::socket(io_service));
		listen_sock.async_accept(*client_sock, yielder[ec]);

		// 进入 client 处理
		if(!ec)
			boost::asio::spawn(io_service, boost::bind(&process_client, _1, client_sock, privatekey, x509_cert));
	}
}

int pass_cb(char *buf, int size, int rwflag, char *u)
{
	int len;
	const char *tmp;
	/* We'd probably do something else if 'rwflag' is 1 */
	printf("Enter pass phrase for \"%s\"\n", u);

	/* get pass phrase, length 'len' into 'tmp' */
	tmp = "route";
	len = strlen(tmp);

	if (len <= 0) return 0;
	/* if too long, truncate */
	if (len > size) len = size;
	memcpy(buf, tmp, len);
	return len;
}

int main()
{
	OpenSSL_add_all_algorithms();

	int port = 24950; // "av" = 0x6176 = 24950
	// 开启 av协议处理

	boost::shared_ptr<BIO> keyfile(BIO_new_file("route.key","r"), BIO_free);
	if(!keyfile)
	{
		std::cerr << "can not open route.key" << std::endl;
		exit(1);
	}

	RSA * rsa_key = PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, (pem_password_cb*) pass_cb, (void*)"route");

	boost::shared_ptr<BIO> certfile(BIO_new_file("route.crt", "r"), BIO_free);
	if(!certfile)
	{
		std::cerr << "can not open route.crt" << std::endl;
		exit(1);
	}
	X509 * x509_cert = PEM_read_bio_X509(certfile.get(), 0, 0, 0);

	certfile.reset();
	keyfile.reset();

	boost::asio::spawn(io_service, boost::bind(&async_acceptor, _1, port, rsa_key, x509_cert));

	// 无限睡眠，客户端的话就开始写客户端的逻辑吧
	io_service.run();
}
