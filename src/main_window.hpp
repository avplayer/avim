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
class Ui_main_window;
class main_window : public QMainWindow
{
	Q_OBJECT
public:
	main_window(QAbstractItemModel* buddy, QAbstractItemModel* group, QAbstractItemModel* recent);

public:
	// 槽, 登录成功调用, GUI 自己会做相应更新
	void on_login_success();

	// 槽, 丢失链接的时候调用, GUI 自己会做相应更新
	void on_lost_connection(int reason);

public:
Q_SIGNALS:
	// 信号, 用户开始和某用户开始交谈的时候发射
	void chat_opened(std::string, bool is_group);
	void close_requested(QCloseEvent*e);

private:
    virtual void closeEvent(QCloseEvent*);
private:

	std::shared_ptr<Ui_main_window> m_ui;
};
