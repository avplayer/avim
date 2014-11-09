
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <avjackif.hpp>
#include <avproto.hpp>

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

// 一个非常非常简单的 IM 实现，测试用途

static boost::asio::io_service io_service;

avkernel avcore(io_service);

// 构造 avtcpif
// 创建一个 tcp 的 avif 设备，然后添加进去
boost::shared_ptr<avjackif> avinterface;

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

static void msg_login_and_send(boost::asio::yield_context yield_context)
{
	std::string me_addr = "test@avplayer.org";

	avinterface->async_handshake(yield_context) &&
	// 添加接口
	avcore.add_interface(avinterface) &&
	// 添加路由表, metric越大，优先级越低
	avcore.add_route(".+@.+", me_addr, avinterface->get_ifname(), 100);

	// 进入 IM 过程，发送一个 test  到 test2@avplayer.org
	boost::async(
		[](){
			avcore.sendto("test-client@avplayer.org", "test, me are testing you stupid avim");
		}
	);

}

int pass_cb(char *buf, int size, int rwflag, char *u)
{
	int len;
	std::string tmp;
	/* We'd probably do something else if 'rwflag' is 1 */
	std::cout << "Enter pass phrase for " << u << " :";
	std::flush(std::cout);

	std::cin >> tmp;

	/* get pass phrase, length 'len' into 'tmp' */
	len = tmp.length();

	if (len <= 0) return 0;
	/* if too long, truncate */
	if (len > size) len = size;
	memcpy(buf, tmp.data(), len);
	return len;
}

// 真 main 在这里, 这里是个臭协程
static void real_main(boost::asio::yield_context yield_context)
{

}

int main(int argc, char * argv[])
{
	OpenSSL_add_all_algorithms();
	fs::path key, cert;

	po::variables_map vm;
	po::options_description desc("qqbot options");
	desc.add_options()
	("key", po::value<fs::path>(&key)->default_value("avim.key"), "path to private key")
	("cert", po::value<fs::path>(&cert)->default_value("avim.cert"), "path to cert")
	("help,h",  "display this help");

	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cerr <<  desc <<  std::endl;
		return 1;
	}

	boost::shared_ptr<BIO> keyfile(BIO_new_file(key.c_str(), "r"), BIO_free);
	if(!keyfile)
	{
		std::cerr <<  desc <<  std::endl;
		std::cerr << "can not open " << key << std::endl;
		exit(1);
	}
	RSA * rsa_key = PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, (pem_password_cb*)pass_cb,(void*) key.c_str());

	boost::shared_ptr<BIO> certfile(BIO_new_file(cert.c_str(), "r"), BIO_free);
	if(!certfile)
	{
		std::cerr <<  desc <<  std::endl;
		std::cerr << "can not open "<< cert << std::endl;
		exit(1);
	}
	X509 * x509_cert = PEM_read_bio_X509(certfile.get(), 0, 0, 0);

	certfile.reset();
	keyfile.reset();

	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::shared_ptr<boost::asio::ip::tcp::socket> avserver( new boost::asio::ip::tcp::socket(io_service));
	boost::asio::connect(*avserver, resolver.resolve(boost::asio::ip::tcp::resolver::query("avim.avplayer.org", "24950")));

	avinterface.reset(new avjackif(avserver) );

	avinterface->set_pki(rsa_key, x509_cert);

	boost::asio::spawn(io_service, boost::bind(&real_main, _1));

	boost::asio::spawn(io_service, msg_login_and_send);

	// 开协程异步接收消息

	boost::asio::spawn(io_service, msg_reader);
	io_service.run();
}
