
#include <cstdlib>
#include <boost/bind.hpp>
#include <openssl/pem.h>

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>

#include "app.hpp"
#include "chat_widget.hpp"

avimApp::avimApp(int argc, char* argv[])
	: QApplication(argc, argv)
	, m_io_work(m_io_service)
	, m_avkernel(m_io_service)
{
	// 开启 boost 线程跑 io_service
	m_io_thread = std::thread([this]()
	{
		try
		{
			m_io_service.run();
		}
		catch (std::exception& e)
		{
			std::cerr << "main exception: " << e.what() << std::endl;
			return -1;
		}
	});

	// 发送一个测试 信号, 看能不能顺利的在2个线程里同步
	qDebug() << "test GUI thread and IO thread interactivity" ;
	m_io_service.post([]()
	{
		std::cerr << "this code run in asio thread : " << std::this_thread::get_id() << std::endl;
		post_on_gui_thread([]()
		{
			std::cerr << "this code run in GUI thread : " << std::this_thread::get_id() << std::endl;
		});
	});
}

avimApp::~avimApp()
{
	m_io_service.stop();
	m_io_thread.join();
}

bool avimApp::load_key_and_cert(std::string cur_key, std::string cur_cert)
{
	if (cur_key.empty())
	{
		fs::path avim_key = get_app_dir();

		avim_key /= "user.key";
		cur_key = avim_key.string();
	}

	if (cur_cert.empty())
	{
		fs::path avim_cert = get_app_dir();

		avim_cert /= "user.cert";
		cur_key = avim_cert.string();
	}

	if (!fs::exists(fs::path(cur_key)) || !fs::exists(fs::path(cur_cert)))
	{
		std::cout << cur_key << '\n';
		std::cout << cur_cert << '\n';
		std::cout << "omg, key and cert not exist";
		return false;
	}
	qDebug() << "cert:" << QString::fromStdString(cur_cert);
	qDebug() << "key:" << QString::fromStdString(cur_key);

	std::shared_ptr<BIO> bio_key {BIO_new_file(cur_key.c_str(), "r") , BIO_free};
	std::shared_ptr<BIO> bio_cert {BIO_new_file(cur_cert.c_str(), "r") , BIO_free};

	auto _c_rsa_key = PEM_read_bio_RSAPrivateKey(bio_key.get(), nullptr, [](char * buf, int size, int rwflag, void * parent)->int
	{
		// 打开窗口, 提升用户输入密码
		bool ok;
		QString text = QInputDialog::getText(
			nullptr,
			tr("unlock key"),
			tr("the private key is password protected, please input the password:"), QLineEdit::Password,
			"",
			&ok
		);

		if (ok && !text.isEmpty())
		{
			strncpy(buf, text.toUtf8().constData(), size);
			return text.toUtf8().length();
		}
		return -1;
	}, (void*) this);

	std::shared_ptr<RSA> user_key(_c_rsa_key, RSA_free);

	if (!user_key)
	{
		// TODO 提示错误, 重新输入
		// 现在暂时暴力退出

		QMessageBox box;
		box.setText("unable to read the private key!");
		box.exec();
		std::exit(1);
	}

	std::shared_ptr<X509> user_cert
	{
		PEM_read_bio_X509(bio_cert.get(), NULL, NULL, NULL),
		X509_free
	};

	// 设置下 cert 和密码
	m_avconnection.reset(new AVConnection(m_io_service));
	m_avconnection->set_cert_and_key(user_key, user_cert);
	return true;
}

int avimApp::exec()
{
	load_cfg();

	std::string auto_login = m_cfg->get<std::string>("global.auto_login");

	if (auto_login == "true")
	{
		if (load_key_and_cert(m_cfg->get<std::string>("global.key"), m_cfg->get<std::string>("global.cert")))
		{
			return start_main();
		}
		else
		{
			m_login_dialog.reset(new login_dialog(m_cfg.get()));
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
		m_login_dialog.reset(new login_dialog(m_cfg.get()));
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

void avimApp::load_cfg()
{
	fs::path appdatadir = get_app_dir();

	if (!fs::exists(appdatadir))
		fs::create_directories(appdatadir);

	m_cfg.reset(new avim::ini(appdatadir / "config.ini"));
}

int avimApp::start_main()
{
	// 创建主窗口, 开始真正的 GUI 之旅
	m_mainwindow.reset(new MainWindow(this));
	connect(this, SIGNAL(login_success()), m_mainwindow.get(), SLOT(on_login_success()), Qt::QueuedConnection);
	connect(m_mainwindow.get(), SIGNAL(chat_opened(std::string)), this, SLOT(start_chat_with(std::string)), Qt::QueuedConnection);

	// 连接成功后马上登录
	connect(m_avconnection.get(), SIGNAL(server_connected()), m_avconnection.get(), SLOT(start_login()), Qt::QueuedConnection);
	// 要登录成功的消息!
	connect(m_avconnection.get(), SIGNAL(login_success()), this, SIGNAL(login_success()), Qt::QueuedConnection);
	connect(m_avconnection.get(), &AVConnection::login_success, std::bind(&AVConnection::handover_to_avkernel, m_avconnection.get(), std::ref(m_avkernel)));

	m_mainwindow->show();
	m_avconnection->start_connect();

	return QApplication::exec();
}

void avimApp::start_chat_with(std::string budy)
{
	QMessageBox box;
	box.setText("别激动, 木实现");
	box.exec();

	// TODO 打开 chat 窗口
	// 别激动, 这里故意内存泄漏的
	auto chat_widet = new avui::chat_widget(budy);
	chat_widet->show();
}

MainWindow::MainWindow(avimApp* _avimapp)
	: m_avimapp(_avimapp)
{
	setCentralWidget(new QWidget);
	auto vlayout = new QVBoxLayout(centralWidget());
	m_list = new QListWidget(centralWidget());
	vlayout->addWidget(m_list);

	m_list->addItem("test@avplayer.org");
	m_list->addItem("test-client@avplayer.org");
	m_list->addItem("microcai@avplayer.org");

	m_list->setEnabled(false);

	connect(m_list, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem * item)
	{
		Q_EMIT chat_opened(item->text().toStdString());
	});
}

void MainWindow::on_login_success()
{
	// TODO 更新 GUI 向用户显示
	m_list->setEnabled(true);
}

void MainWindow::on_lost_connection(int reason)
{
	// TODO 更新 GUI 报告用户
}
