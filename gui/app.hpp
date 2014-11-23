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


#include <avim.hpp>

#include "avim.h"
#include "login_dialog.h"


class MainWindow : public QMainWindow
{
public:
	MainWindow()
	{
		m_list = new QListWidget(this);
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

	int exec()
	{
		load_cfg();

		std::string auto_login = m_cfg->get<std::string>("global.auto_login");

		if (auto_login == "true")
		{
			if (load_key_and_cert(m_cfg->get<std::string>("global.key"), m_cfg->get<std::string>("global.cert")))
			{
				return QApplication::exec();
			}
			else
			{
				m_login_dialog.reset(new login_dialog);
				if (m_login_dialog->exec() == QDialog::Accepted)
				{
					if (!load_key_and_cert(m_login_dialog->get_key_path(), m_login_dialog->get_cert_path()))
					{
						return 1;
					}
					m_login_dialog.reset();
					return start_main();
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			m_login_dialog.reset(new login_dialog);
			if (m_login_dialog->exec() == QDialog::Accepted)
			{
				if (!load_key_and_cert(m_login_dialog->get_key_path(), m_login_dialog->get_cert_path()))
				{
					return 1;
				}
				m_login_dialog.reset();
				return start_main();
			}
			else
			{
				return 0;
			}
		}
	}

	void load_cfg()
	{
		fs::path appdatadir = get_app_dir();

		if (!fs::exists(appdatadir))
			fs::create_directories(appdatadir);

		m_cfg.reset(new avim::ini(appdatadir / "config.ini"));
	}

    bool load_key_and_cert(std::string cur_key, std::string cur_cert);

protected:
	int start_main()
	{
		// 创建主窗口, 开始真正的 GUI 之旅
		m_mainwindow.reset(new MainWindow);
		m_mainwindow->show();
		return QApplication::exec();
	}
};

