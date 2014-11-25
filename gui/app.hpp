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

#include <avim.hpp>

#include "avim.h"
#include "login_dialog.h"
#include "avconnection.hpp"

void post_on_gui_thread(std::function<void()>);

class avimApp;
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(avimApp* _avimapp);

private Q_SLOTS:

	// 槽, 登录成功调用, GUI 自己会做相应更新
	void on_login_success();

	// 槽, 丢失链接的时候调用, GUI 自己会做相应更新
	void on_lost_connection(int reason);

Q_SIGNALS:
	// 信号, 用户开始和某用户开始交谈的时候发射
	void chat_opened(std::string);

private:
	QListWidget* m_list;

	avimApp* m_avimapp;

	friend class avimApp;
};

class avimApp : public QApplication
{
Q_OBJECT

private Q_SLOTS:
	void start_chat_with(std::string);
public:
	avimApp(int argc, char *argv[]);

    ~avimApp();
public:
	Q_SIGNALS:

	// 信号,登录成功发射
	void login_success();

	// 信号, 丢失链接的时候发射
	void connection_lost(int reason);

public:
	fs::path get_app_dir()
	{
		return QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();
	}

	int exec();

public Q_SLOTS:
	void load_cfg();

    bool load_key_and_cert(std::string cur_key, std::string cur_cert);

protected:
	int start_main();

private:
	boost::asio::io_service m_io_service;
	avkernel m_avkernel;
	std::unique_ptr<AVConnection> m_avconnection;
	std::unique_ptr<login_dialog> m_login_dialog;
	std::unique_ptr<MainWindow> m_mainwindow;
	std::list<QWidget*> m_chats;

	std::thread m_io_thread;
	boost::asio::io_service::work m_io_work;

	std::unique_ptr<avim::ini> m_cfg;

	friend class MainWindow;
};

