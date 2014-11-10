
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <avif.hpp>
#include <avjackif.hpp>
#include <avproto.hpp>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

/*
 * 这个文件提供的是 C 接口，如果使用 C++，强烈的建议使用 aiso 和 C++ 接口！也就是直接使用 avkernel 对象！
 *
 */

static boost::asio::io_service * av_service;
static avkernel * avkernel_intance;
static boost::thread * avkernelthread;

static void create_avkernel_instance()
{
	av_service = new boost::asio::io_service;
	avkernel_intance = new avkernel(*av_service);

	avkernelthread = new boost::thread(boost::bind(&boost::asio::io_service::run, av_service));
}

void av_start()
{
	static boost::once_flag onceflag;
	boost::call_once(onceflag, create_avkernel_instance);
}

void av_stop()
{
	av_service->stop();
	avkernelthread->join();
	delete avkernel_intance;
	delete av_service;
	delete avkernelthread;
}

static void jackif_login(boost::shared_ptr<avjackif> avinterface, boost::asio::yield_context yield_context, boost::condition_variable & ready)
{
	std::string me_addr = av_address_to_string(*avinterface->if_address());

	avinterface->async_handshake(yield_context) &&
	// 添加接口
	avkernel_intance->add_interface(avinterface) &&

	// 添加路由表, metric越大，优先级越低
	avkernel_intance->add_route(".+@.+", me_addr, avinterface->get_ifname(), 100);

	// 返回
	ready.notify_all();
}

// FIXME 添加错误处理
int connect_to_avrouter(const char * keyfilename, const char * certfilename, const char * self_addr, const char * host, const char * port)
{
	boost::shared_ptr<BIO> keyfile(BIO_new_file(keyfilename, "r"), BIO_free);
	if(!keyfile)
	{
		std::cerr << "can not open avim.key" << std::endl;
		exit(1);
	}
	RSA * rsa_key = PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, 0, 0);

	boost::shared_ptr<BIO> certfile(BIO_new_file(certfilename, "r"), BIO_free);
	if(!certfile)
	{
		std::cerr << "can not open avim.crt" << std::endl;
		exit(1);
	}
	X509 * x509_cert = PEM_read_bio_X509(certfile.get(), 0, 0, 0);

	certfile.reset();
	keyfile.reset();

	boost::asio::ip::tcp::resolver resolver(*av_service);

	boost::shared_ptr<boost::asio::ip::tcp::socket> avserver( new boost::asio::ip::tcp::socket(*av_service));

	// 连接
	boost::asio::connect(*avserver, resolver.resolve(boost::asio::ip::tcp::resolver::query(host, port ? port : "24950")));

	// 构造 avtcpif
	// 创建一个 tcp 的 avif 设备，然后添加进去
	boost::shared_ptr<avjackif> avinterface(new avjackif(avserver));

	boost::mutex m;
	boost::condition_variable ready;
	boost::unique_lock<boost::mutex> l(m);
 	boost::asio::spawn(*av_service, boost::bind(&jackif_login, avinterface, _1, boost::ref(ready)));
	ready.wait(l);
}

int av_sendto(const char * dest_address, const char * message, int len)
{
	return avkernel_intance->sendto(dest_address, std::string(message, len));
}

int av_recvfrom(char * dest_address, char * message)
{
	std::string dest, msg;
	auto ret = avkernel_intance->recvfrom(dest, msg);
	strcpy(dest_address, dest.c_str());
	memcpy(message, msg.c_str(), msg.length());
	return ret;
}
