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

class avimApp;
class main_window : public QMainWindow
{
	Q_OBJECT
public:
	main_window(avimApp* _avimapp);

private Q_SLOTS:

	// 槽, 登录成功调用, GUI 自己会做相应更新
	void on_login_success();

	// 槽, 丢失链接的时候调用, GUI 自己会做相应更新
	void on_lost_connection(int reason);

public:
Q_SIGNALS:
	// 信号, 用户开始和某用户开始交谈的时候发射
	void chat_opened(std::string);

private:
	QListWidget* m_list;

	avimApp* m_avimapp;

	friend class avimApp;
};
