#pragma once

#include <memory>
#include <boost/noncopyable.hpp>

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

	};

public:
    AVConnection();
Q_SIGNALS:
	void connection_state_change(ConState);

public Q_SLOTS:
	void set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert);

private:

	std::shared_ptr<RSA> m_key;
	std::shared_ptr<X509> m_cert;
	std::shared_ptr<avjackif> m_avif;
};
