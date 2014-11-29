
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/bind.hpp>
#include <QMetaType>
#include <QtGlobal>
#include <QDebug>

#include "avproto.hpp"

#include "avconnection.hpp"

Q_DECLARE_METATYPE(std::shared_ptr<RSA>);
Q_DECLARE_METATYPE(std::shared_ptr<X509>);
Q_DECLARE_METATYPE(AVConnection::ConState);

AVConnection::AVConnection(boost::asio::io_service& _io)
	: m_io_service(_io)
{
}

void AVConnection::set_state(AVConnection::ConState _s)
{
	if (m_state != _s)
	{
		m_state = _s;
		Q_EMIT connection_state_change(m_state);
	}
}

void AVConnection::set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert)
{
	m_key = key;
	m_cert = cert;
}

void AVConnection::start_connect()
{
	// 检查 cert 和 key 是否已经设置, 没有设置的话就错误
	if (!m_key || !m_cert)
	{
		return Q_EMIT invalid_cert();
	}

	// 创建协程干活
	boost::asio::spawn(m_io_service, std::bind(&AVConnection::connect_coroutine, this, std::placeholders::_1));
}

void AVConnection::start_login()
{
	Q_ASSERT(m_state == CONNECTED);
	// 创建协程干活
	m_avif.reset(new avjackif(m_socket));
	boost::asio::spawn(m_io_service, std::bind(&AVConnection::login_coroutine, this, std::placeholders::_1));
}

void AVConnection::handover_to_avkernel(avkernel& kernel)
{
	if (kernel.add_interface(m_avif))
	{
		m_avif->signal_notify_remove.connect([this](){
			set_state(DISCONNECTED);
			Q_EMIT interface_removed();
		});

		std::string me_addr = av_address_to_string(*m_avif->if_address());

		// 添加路由表, metric越大，优先级越低
		kernel.add_route(".+@.+", me_addr, m_avif->get_ifname(), 100);

		Q_EMIT kernel_accepted();
	}
}

void AVConnection::connect_coroutine(boost::asio::yield_context yield_context)
{
	boost::system::error_code ec;
	// 传教到 avim.avplayer.org 的连接
	using namespace boost::asio::ip;

	tcp::resolver resolver(get_io_service());

	auto _debug_host = getenv("AVIM_HOST");
	tcp::resolver::query query(_debug_host?_debug_host:"avim.avplayer.org", "24950");

	auto endpoint_iterator = resolver.async_resolve(query, yield_context[ec]);

	if (ec || endpoint_iterator == tcp::resolver::iterator())
	{
		return Q_EMIT server_not_found();
	}

	m_socket.reset(new tcp::socket(get_io_service()));

	boost::asio::async_connect(*m_socket, endpoint_iterator, yield_context[ec]);

	if (ec)
	{
		qDebug() << QString(ec.message().c_str());
		return Q_EMIT connection_refused();
	}

	Q_EMIT server_connected();
	set_state(CONNECTED);
}

void AVConnection::login_coroutine(boost::asio::yield_context yield_context)
{
	m_avif->set_pki(m_key, m_cert);

	if (m_avif->async_handshake(yield_context))
	{
		// 成功
		Q_EMIT login_success();
		set_state(AUTHORIZED);
	}
}
