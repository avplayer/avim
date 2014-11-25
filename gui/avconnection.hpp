#pragma once

#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <QObject>

class avjackif;
class AVConnection :  public QObject,  boost::noncopyable
{
	Q_OBJECT
public:
	enum ConState{
		DISCONNECTED,
		CONNECTED,
		AUTHORIZED,
		CONNECTIONRESET,
	};

public:
    AVConnection(boost::asio::io_service&);
Q_SIGNALS:
	void connection_state_change(ConState);
	void invalid_cert();
	void cert_accepted();
	// 服务器没找到, 检查 DNS 设置或者网络连接
	void server_not_found();

	// 连接被拒绝
	void connection_refused();
	void server_connected();

	void login_success();

public Q_SLOTS:
	void set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert);

	void start_connect();
	void start_login();

	boost::asio::io_service& get_io_service(){return m_io_service;}

private: // coroutines
	void connect_coroutine(boost::asio::yield_context);
	void login_coroutine(boost::asio::yield_context);

private:
	void set_state(ConState);
	ConState m_state;
	boost::asio::io_service& m_io_service;
	std::shared_ptr<RSA> m_key;
	std::shared_ptr<X509> m_cert;
	std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
	std::shared_ptr<avjackif> m_avif;
};
