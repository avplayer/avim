#include <string>
#include <iostream>
#include <fstream>
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
#include <message.hpp>
#include <avim.hpp>

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

// 一个非常非常简单的 IM 实现，测试用途

static boost::asio::io_service io_service;

boost::scoped_ptr<avim_client> avim;

static void msg_reader(boost::asio::yield_context yield_context)
{
	boost::system::error_code ec;
	proto::av_address sender;
	proto::avim_message_packet msgpkt;

	for (;;)
	{
		avim->async_recv_im([](proto::av_address) {return false;}, sender, msgpkt, yield_context);

		std::cerr << "接收到的数据： " << av_address_to_string(sender) << "说: ";

		for (proto::avim_message im_message_item : msgpkt.avim())
		{
			if (im_message_item.has_item_text())
			{
				std::cerr << im_message_item.item_text().text() << std::endl;
			}
		}

		std::cerr << std::endl;
	}
}

static void msg_login_and_send(std::string to, boost::asio::yield_context yield_context)
{
	avim->async_wait_online(yield_context);

	std::string msg =std::string("test, me are sending a test message to ") + to + " stupid!";

	proto::avim_message_packet msgpkt;
	msgpkt.mutable_avim()->Add()->mutable_item_text()->set_text(msg);

	if (to.empty())
	{
		avim->async_send_im(avim->self_address(), msgpkt, yield_context);
	}
	else
	{
		// 进入 IM 过程，发送一个 test  到 test2@avplayer.org
		avim->async_send_im(av_address_from_string(to), msgpkt, yield_context);
	}
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

int main(int argc, char * argv[])
{
	OpenSSL_add_all_algorithms();
	fs::path key, cert;

	std::string to;

	po::variables_map vm;
	po::options_description desc("qqbot options");
	desc.add_options()
	("key", po::value<fs::path>(&key)->default_value("avim.key"), "path to private key")
	("cert", po::value<fs::path>(&cert)->default_value("avim.cert"), "path to cert")
	("help,h", "display this help")
	("to",po::value<std::string>(&to), "send test message to, default to send to your self");

	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cerr << desc << std::endl;
		return 1;
	}

	if (!fs::exists(key))
	{
		std::cerr <<  desc <<  std::endl;
		std::cerr << "can not open " << key << std::endl;
		exit(1);
	}
	if (!fs::exists(cert))
	{
		std::cerr <<  desc <<  std::endl;
		std::cerr << "can not open " << cert << std::endl;
		exit(1);
	}

	std::string keyfilecontent, keyfilecontent_decrypted, certfilecontent;

	{
		std::ifstream keyfile(key.string().c_str(), std::ios_base::binary | std::ios_base::in);
		std::ifstream certfile(cert.string().c_str(), std::ios_base::binary | std::ios_base::in);
		keyfilecontent.resize(fs::file_size(key));
		certfilecontent.resize(fs::file_size(cert));
		keyfile.read(&keyfilecontent[0], fs::file_size(key));
		certfile.read(&certfilecontent[0], fs::file_size(cert));
	}

	// 这里通过读取然后写回的方式预先将私钥的密码去除

	boost::shared_ptr<BIO> keyfile(BIO_new_mem_buf(&keyfilecontent[0], keyfilecontent.length()), BIO_free);
	boost::shared_ptr<RSA> rsa_key(
		PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, (pem_password_cb*)pass_cb,(void*) key.c_str()),
		RSA_free
	);

	keyfile.reset(BIO_new(BIO_s_mem()), BIO_free);
	char *outbuf = 0;
	PEM_write_bio_RSAPrivateKey(keyfile.get(),rsa_key.get(), 0, 0, 0, 0, 0);
	rsa_key.reset();
	auto l = BIO_get_mem_data(keyfile.get(), &outbuf);
	keyfilecontent.assign(outbuf, l);
	keyfile.reset();

	// 读入 key 和 cert 的内容
	avim.reset(new avim_client(io_service, keyfilecontent, certfilecontent));

	boost::asio::spawn(io_service, boost::bind(&msg_login_and_send, to, _1));

	// 开协程异步接收消息
	boost::asio::spawn(io_service, msg_reader);
	io_service.run();
}
