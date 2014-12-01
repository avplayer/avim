
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

void AVConnection::start_login()
{
	// 检查 cert 和 key 是否已经设置, 没有设置的话就错误
	if (!m_key || !m_cert)
	{
		return Q_EMIT invalid_cert();
	}

	Q_ASSERT(m_state == DISCONNECTED);
	// 创建协程干活
	m_avif.reset(new avjackif(m_io_service));
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

void AVConnection::login_coroutine(boost::asio::yield_context yield_context)
{
	m_avif->set_pki(m_key, m_cert);
	auto _debug_host = getenv("AVIM_HOST");

	if (m_avif->async_handshake(_debug_host?_debug_host:"avim.avplayer.org", "24950", yield_context))
	{
		// 成功
		Q_EMIT login_success();
		set_state(AUTHORIZED);
	}
}
