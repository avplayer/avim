
#include <boost/asio/spawn.hpp>
#include <boost/bind.hpp>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

#include "avim.hpp"
#include "avjackif.hpp"
#include "message.hpp"

avim_client::avim_client(boost::asio::io_service& io, std::string privatekey, std::string public_cert)
	: io_service(io)
	, m_avkernel(io_service)
	, m_online(false)
{
	// 读取 RSA
	boost::shared_ptr<BIO> keyfile(BIO_new_mem_buf(&privatekey[0], privatekey.length()), BIO_free);
	boost::shared_ptr<BIO> certfile(BIO_new_mem_buf(&public_cert[0], public_cert.length()), BIO_free);

	m_rsa_key.reset(
		PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, 0, 0), //(pem_password_cb*)pass_cb,(void*) key.c_str()),
		RSA_free
	);

	m_x509_cert.reset(PEM_read_bio_X509(certfile.get(), 0, 0, 0), X509_free);

	boost::asio::spawn(io_service, boost::bind(&avim_client::coroutine_login, this, _1));
}

void avim_client::coroutine_login(boost::asio::yield_context yield_context)
{
	boost::asio::ip::tcp::resolver resolver(io_service);
	m_socket.reset( new boost::asio::ip::tcp::socket(io_service));

	auto resolved_host_iterator = resolver.async_resolve(boost::asio::ip::tcp::resolver::query("avim.avplayer.org", "24950"), yield_context);

	boost::asio::async_connect(*m_socket, resolved_host_iterator, yield_context);

	m_avinterface.reset(new avjackif(m_socket) );

	m_avinterface->set_pki(m_rsa_key, m_x509_cert);

	if( m_avinterface->async_handshake(yield_context) && m_avkernel.add_interface(m_avinterface) )
	{
		std::string me_addr = av_address_to_string(*m_avinterface->if_address());

		// 添加路由表, metric越大，优先级越低
		m_avkernel.add_route(".+@.+", me_addr, m_avinterface->get_ifname(), 100);

		m_online = true;
	}
}

void avim_client::async_wait_online(boost::asio::yield_context yield_context)
{
	while(!m_online)
	{
		boost::asio::deadline_timer timer(io_service);
		timer.expires_from_now(boost::posix_time::seconds(2));
		timer.async_wait(yield_context);
	}
}

void avim_client::async_wait_online(boost::function<void()> handler)
{
	boost::asio::spawn(io_service, [this, handler](boost::asio::yield_context yield_context){
		async_wait_online(yield_context);
		handler();
	});
}

proto::av_address avim_client::self_address()
{
	proto::av_address ret;
	ret.CopyFrom(*m_avinterface->if_address());
	return ret;
}

void avim_client::async_recv_im(avim_client::SelectDecryptKeyCallbackType, proto::av_address& sender, proto::avim_message_packet& pkt, boost::asio::yield_context yield_context)
{
	async_wait_online(yield_context);

	std::string target;
	std::string data;
	m_avkernel.async_recvfrom(target, data, yield_context);

	sender = av_address_from_string(target);

	pkt = decode_message(data);
}

void avim_client::async_send_im(const proto::av_address& target, const proto::avim_message_packet& pkt, avim_client::SendHandlerType handler)
{
	async_wait_online([this, target, pkt, handler](){
		std::string data = encode_message(pkt);

		m_avkernel.async_sendto(av_address_to_string(target), data, handler);
	});
}

void avim_client::async_send_im(const proto::av_address& target, const proto::avim_message_packet& pkt, boost::asio::yield_context yield_context)
{
	async_wait_online(yield_context);

	std::string data = encode_message(pkt);

	m_avkernel.async_sendto(av_address_to_string(target), data, yield_context);
}


