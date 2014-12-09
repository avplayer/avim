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
#include "im.pb.h"

#include "chatwidget.hpp"
#include "login_dialog.hpp"
#include "avconnection.hpp"
#include "main_window.hpp"
#include "system_tray.hpp"
#include "buddymodel.hpp"

void post_on_gui_thread(std::function<void()>);

class avimApp : public QApplication
{
Q_OBJECT

private Q_SLOTS:
	avui::ChatWidget* start_chat_with(std::string, bool);
public:
	avimApp(int argc, char *argv[]);

    ~avimApp();

Q_SIGNALS:

	// 好友列表加载完毕后发射
	void buddy_list_loaded();

	// 信号,登录成功发射
	void login_success();

	// 信号, 丢失链接的时候发射
	void connection_lost(int reason);
private:

	void recive_coroutine(boost::asio::yield_context);

	Q_SIGNALS:
	// 收到消息的时候发射!
	void message_recieved(std::string sender, im_message);
	void raw_message_recieved(std::string sender, std::string data);

public:
	fs::path get_app_dir()
	{
		return QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();
	}

	QIcon get_icon();

	int exec();

public Q_SLOTS:

	void load_cfg();

	void do_register_user(QString user, QString mailaddr, QString phone);

	void login_dialog_accepted();

    bool load_key_and_cert(std::string cur_key, std::string cur_cert);

	void send_im_message(std::string target, message::message_packet);
	void send_group_message(std::string target, message::message_packet);
	void send_raw_message(std::string target, std::string);
	void on_message_recieve(std::string target, im_message);
	void on_login_success();

protected Q_SLOTS:
	void start_main();

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

    std::string m_self_addr;

	std::shared_ptr<std::vector<avbuddy>> m_buddy = std::make_shared<std::vector<avbuddy>>();
	std::shared_ptr<std::vector<avbuddy>> m_group = std::make_shared<std::vector<avbuddy>>();
	std::shared_ptr<std::vector<avbuddy>> m_recent = std::make_shared<std::vector<avbuddy>>();

	BuddyModel m_buddy_model;
	BuddyModel m_group_model;
	BuddyModel m_recent_model;

	std::map<avbuddy, std::shared_ptr<std::vector<avbuddy>>> m_members_of_group;

	friend class main_window;
};

