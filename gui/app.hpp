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

void post_on_gui_thread(std::function<void()>);

class MainWindow : public QMainWindow
{
public:
	MainWindow()
	{
		setCentralWidget(new QWidget);
		auto vlayout = new QVBoxLayout(centralWidget());
		m_list = new QListWidget(centralWidget());
		vlayout->addWidget(m_list);

		m_list->addItem("test@avplayer.org");
		m_list->addItem("test-client@avplayer.org");
		m_list->addItem("microcai@avplayer.org");
	}

private:
	QListWidget* m_list;
};

class avimApp : public QApplication
{
Q_OBJECT
private:
	std::unique_ptr<MainWindow> m_mainwindow;

	std::unique_ptr<login_dialog> m_login_dialog;

	std::list<QWidget*> m_chats;

	boost::asio::io_service m_io_service;
	std::thread m_io_thread;
	boost::asio::io_service::work m_io_work;

	std::unique_ptr<avim::ini> m_cfg;

	std::unique_ptr<avim_client> m_avim_client;

private slots:
	void on_post(std::function<void()> qfunc_ptr);

signals:

	void post(std::function<void()>);

public:
	avimApp(int argc, char *argv[]);

    ~avimApp()
	{
		m_io_service.stop();
		m_io_thread.join();
	}

	fs::path get_app_dir()
	{
		return QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();
	}

	int exec();

	void load_cfg();

    bool load_key_and_cert(std::string cur_key, std::string cur_cert);

protected:
	int start_main();
};

