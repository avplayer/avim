#pragma once

#include <memory>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/asio.hpp>

#include <QStandardPaths>
#include <QSharedPointer>
#include <QApplication>
#include <QMainWindow>
#include <QWindow>
#include <QListWidget>
#include <QDialog>

#include "avproto.hpp"
#include "avim_proto/im.pb.h"

#include "chat_widget.hpp"
#include "login_dialog.h"
#include "avconnection.hpp"
#include "main_window.hpp"
#include "system_tray.hpp"

void post_on_gui_thread(std::function<void()>);

class avimApp : public QApplication
{
Q_OBJECT

private Q_SLOTS:
	avui::chat_widget* start_chat_with(std::string);
public:
	avimApp(int argc, char *argv[]);

    ~avimApp();
public:
	Q_SIGNALS:

	// 信号,登录成功发射
	void login_success();

	// 信号, 丢失链接的时候发射
	void connection_lost(int reason);
private:

	void recive_coroutine(boost::asio::yield_context);

	Q_SIGNALS:
	// 收到消息的时候发射!
	void message_recieved(std::string target, proto::avim_message_packet);

public:
	fs::path get_app_dir()
	{
		return QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();
	}

	int exec();

public Q_SLOTS:
	void load_cfg();

    bool load_key_and_cert(std::string cur_key, std::string cur_cert);

	void send_message(std::string target, proto::avim_message_packet);
	void on_message_recieve(std::string target, proto::avim_message_packet);

protected:
	int start_main();

private:
	boost::asio::io_service m_io_service;
	avkernel m_avkernel;
	std::unique_ptr<AVConnection> m_avconnection;
	std::unique_ptr<login_dialog> m_login_dialog;
	std::unique_ptr<main_window> m_mainwindow;
	std::unique_ptr<avim_system_tray> m_tray_icon;
	std::map<std::string, QWidget*> m_chats;

	std::thread m_io_thread;
	boost::asio::io_service::work m_io_work;

	std::unique_ptr<avim::ini> m_cfg;

	friend class main_window;
};

