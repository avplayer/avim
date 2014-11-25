#pragma once

#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>

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

public Q_SLOTS:
	void set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert);

	void start();
	void stop();

	boost::asio::io_service& get_io_service(){return m_io_service;}
private:

	void coroutine(boost::asio::yield_context);

private:
	boost::asio::io_service& m_io_service;
	std::shared_ptr<RSA> m_key;
	std::shared_ptr<X509> m_cert;
	std::shared_ptr<avjackif> m_avif;
};
