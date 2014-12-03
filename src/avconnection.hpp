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
class avkernel;

class AVConnection :  public QObject,  boost::noncopyable
{
	Q_OBJECT
public:
	enum ConState
	{
		DISCONNECTED,
		CONNECTED,
		AUTHORIZED,
		CONNECTIONRESET,
	};

public:
	AVConnection(boost::asio::io_service&);
Q_SIGNALS:
	void connection_state_change(ConState);

	// 没 cert 就登录
	void invalid_cert();
	// 服务器没找到, 检查 DNS 设置或者网络连接
	void server_not_found();

	// 登录成功
	void login_success();

	void login_failed(int);

	// 内核接受了该连接, 此连接已经可以正式使用了
	void kernel_accepted();

	// 内核删除了本连接. 本连接可能出错了, 解决问题后重新添加
	void interface_removed();

public Q_SLOTS:
	void set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert);

	void start_login();

	void handover_to_avkernel(avkernel&);

	boost::asio::io_service& get_io_service() {return m_io_service;}

private: // coroutines
	void login_coroutine(boost::asio::yield_context);

private:
	void set_state(ConState);
	ConState m_state;
	boost::asio::io_service& m_io_service;
	std::shared_ptr<RSA> m_key;
	std::shared_ptr<X509> m_cert;
	std::shared_ptr<avjackif> m_avif;
};
