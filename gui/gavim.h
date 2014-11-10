#ifndef GAVIM_H
#define GAVIM_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QWidget>
#include <QtCore/QThread>
#include "ui_gavim.h"

#include <boost/asio.hpp>

#include <avjackif.hpp>
#include <avproto.hpp>

class recvThread : public QThread
{
	Q_OBJECT
public:
	recvThread(boost::asio::io_service& ios, avkernel& avk, std::string keyfile, std::string certfile) :
		QThread(), io_service_(ios), avcore_(avk), m_keyfile(keyfile), m_certfile(certfile){}
	~recvThread();
	void run();
	void recv_msg(boost::asio::yield_context);
private:
	boost::asio::io_service& io_service_;
	avkernel& avcore_;
	std::string m_keyfile;
	std::string m_certfile;
signals:
	void recvReady(const QString &, const QString &);
};

class gavim : public QWidget
{
	Q_OBJECT

public:
	gavim(QWidget *parent = 0);
	~gavim();

	QString getMessage();
private:
	Ui::gavimClass ui;
	boost::asio::io_service io_service_;
	avkernel avcore_;
	//recvThread rv_thread_;

private slots:
	void on_sendButton_clicked();
	void on_exitButton_clicked();
	void recvHandle(const QString &, const QString &);
};

#endif // GAVIM_H
