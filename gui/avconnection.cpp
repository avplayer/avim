
#include <boost/asio/spawn.hpp>
#include <boost/bind.hpp>
#include <QMetaType>
#include "avconnection.hpp"

Q_DECLARE_METATYPE(std::shared_ptr<RSA>);
Q_DECLARE_METATYPE(std::shared_ptr<X509>);
Q_DECLARE_METATYPE(AVConnection::ConState);

AVConnection::AVConnection(boost::asio::io_service& _io)
	: m_io_service(_io)
{
}

void AVConnection::set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert)
{

}

void AVConnection::start()
{
	// 检查 cert 和 key 是否已经设置, 没有设置的话就错误
	if (!m_key || !m_cert)
	{
		return Q_EMIT invalid_cert();
	}

	// 创建协程干活
	boost::asio::spawn(m_io_service, std::bind(&AVConnection::coroutine, this, std::placeholders::_1));
}

void AVConnection::stop()
{

}

void AVConnection::coroutine(boost::asio::yield_context yield_context)
{

}
