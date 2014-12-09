
#include <cstdlib>
#include <boost/bind.hpp>
#include <openssl/pem.h>

#include <QDebug>
#include <QTimer>
#include <QInputDialog>
#include <QMessageBox>
#include <QCloseEvent>

#include "avproto.hpp"
#include "app.hpp"
#include "chatwidget.hpp"
#include "system_tray.hpp"

#include "group.pb.h"

avimApp::avimApp(int argc, char* argv[])
	: QApplication(argc, argv)
	, m_io_work(m_io_service)
	, m_avkernel(m_io_service)
	, m_buddy_model(m_buddy)
	, m_group_model(m_group)
	, m_recent_model(m_recent)
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
			std::exit(1);
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

	connect(this, &avimApp::message_recieved, this, &avimApp::on_message_recieve, Qt::QueuedConnection);

	// FIXME
	// 作为测试用途, 来, 添加点什么数据进去

	m_buddy->push_back("microcai@avplayer.org");
	m_buddy->push_back("test-client@avplayer.org");
	m_buddy->push_back("michael.fan@avplayer.org");

	m_group->push_back("group@avplayer.org");
}

avimApp::~avimApp()
{
	m_io_service.stop();
	m_io_thread.join();
}

QIcon avimApp::get_icon()
{
	QIcon ico;
#ifndef _WIN32
	ico = QIcon(":/avim/logo.svg");
#else
	extern QPixmap qt_pixmapFromWinHICON(HICON);
	HICON hicon = (HICON)::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(1),
		IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_LOADTRANSPARENT);
	ico = QIcon(qt_pixmapFromWinHICON(hicon));
	::DestroyIcon(hicon);
#endif
	return ico;
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

void avimApp::login_dialog_accepted()
{
	if (!load_key_and_cert(m_login_dialog->get_key_path().toStdString(), m_login_dialog->get_cert_path().toStdString()))
	{
		QApplication::quit();
	}
	m_login_dialog.reset();
	start_main();
}

void avimApp::do_register_user(QString user, QString mailaddr, QString phone)
{
	if (!m_avconnection)
		m_avconnection.reset(new AVConnection(m_io_service));
	m_avconnection->do_register_user(user.toStdString(), mailaddr.toStdString(), phone.toStdString(), [](int code)
	{
		post_on_gui_thread([code]()
		{
			// 处理注册结果
		});
	});
}

int avimApp::exec()
{
	setWindowIcon(get_icon());
	load_cfg();

	auto auto_login = QSettings().value("global/auto_login").toBool();

	m_login_dialog.reset(new login_dialog());
	QObject::connect(m_login_dialog.get(), SIGNAL(accepted()), this, SLOT(login_dialog_accepted()));
	QObject::connect(m_login_dialog.get(), &login_dialog::request_registering, this, &avimApp::do_register_user);

	if (auto_login)
	{
		QSettings cfg;
		if (load_key_and_cert(cfg.value("global/key").toString().toStdString(), cfg.value("global/cert").toString().toStdString()))
		{
			m_login_dialog.reset();
			QTimer::singleShot(0, this, SLOT(start_main()));
		}
		else
		{
			m_login_dialog->show();
		}
	}
	else
	{
		m_login_dialog->show();
	}

	return QApplication::exec();
}

void avimApp::load_cfg()
{
	fs::path appdatadir = get_app_dir();

	if (!fs::exists(appdatadir))
		fs::create_directories(appdatadir);
}

void avimApp::start_main()
{
	// 创建主窗口, 开始真正的 GUI 之旅
	m_mainwindow.reset(new main_window(&m_buddy_model, &m_group_model, &m_recent_model));
	connect(this, &avimApp::login_success, m_mainwindow.get(), &main_window::on_login_success, Qt::QueuedConnection);
	connect(m_mainwindow.get(), &main_window::chat_opened, this, &avimApp::start_chat_with, Qt::QueuedConnection);
	connect(m_mainwindow.get(), &main_window::close_requested, this, [this](QCloseEvent*e)
	{
		if (m_chats.empty())
		{
			e->accept();
		}else
		{
			e->ignore();

			m_mainwindow->setWindowState(Qt::WindowMinimized);
		}
	});
	m_mainwindow->show();

	// 要登录成功的消息!
	connect(m_avconnection.get(), &AVConnection::login_success, this, &avimApp::login_success, Qt::QueuedConnection);
	connect(m_avconnection.get(), &AVConnection::login_success, m_avconnection.get(), std::bind(&AVConnection::handover_to_avkernel, m_avconnection.get(), std::ref(m_avkernel)));
	connect(this, &avimApp::login_success, [this]()
	{
		m_self_addr = m_avconnection->get_self_addr();
	});

	m_avconnection->start_login();
	connect(m_avconnection.get(), &AVConnection::interface_removed, m_avconnection.get(), std::bind(&AVConnection::start_login, m_avconnection.get()));

	// 开启消息接收协程
	boost::asio::spawn(m_io_service, std::bind(&avimApp::recive_coroutine, this, std::placeholders::_1));

	// 显示 tray icon
	m_tray_icon.reset(new avim_system_tray(get_icon()));
	m_tray_icon->show();

	connect(m_tray_icon.get(), SIGNAL(menu_request_quit()), this, SLOT(quit()));
}

void avimApp::recive_coroutine(boost::asio::yield_context yield_context)
{
	for(;;)
	{
		std::string target,data;

		qRegisterMetaType<im_message>("im_message");

		m_avkernel.async_recvfrom(target, data, yield_context);

		try
		{
			if (is_control_message(data))
			{
				post_on_gui_thread([this, target, data](){
					Q_EMIT raw_message_recieved(target, data);
				});
			}
			else
			{
				post_on_gui_thread([this, target, data](){
					// TODO 看是否是群消息, 如果是, 查找对应的群 key 并解码
					// 解码后再调用重载的另一个版本 decode_im_message
					try{
					Q_EMIT message_recieved(target, decode_im_message(data));
					}catch(const std::runtime_error&)
					{
						qDebug() << "invalid packet recived ";
					}
				});
			}

		}catch (std::exception&)
		{
		}
	}
}

avui::ChatWidget* avimApp::start_chat_with(std::string target, bool is_group)
{
	// 先找下是否已经有窗口打开了, 直接激活

	auto chat_widget_it = m_chats.find(target);
	if (chat_widget_it != m_chats.end())
	{
		QWidget * w = chat_widget_it->second;
		w->activateWindow();
		return (avui::ChatWidget*) w;
	}

	if( m_members_of_group.find(target) == m_members_of_group.end())
	{
		m_members_of_group.insert(std::make_pair(avbuddy(target), std::make_shared<std::vector<avbuddy>>()));
	}

	auto group_data = m_members_of_group[target];
	auto group_data_model = new BuddyModel(group_data);
	// 打开 chat 窗口
	auto chat_widget = new avui::ChatWidget(target, group_data_model);
	chat_widget->show();

	if (is_group)
		connect(chat_widget, &avui::ChatWidget::send_message, std::bind(&avimApp::send_group_message, this, target, std::placeholders::_1));
	else
		connect(chat_widget, &avui::ChatWidget::send_message, std::bind(&avimApp::send_im_message, this, target, std::placeholders::_1));

	// 如果是个群聊, 开始刷群列表
	if (is_group)
	{
		proto::group::list_request list_request;
		list_request.set_id(1);
		auto raw = encode_control_message(list_request);
		send_raw_message(target, raw);

		QMetaObject::Connection slot_connect = QObject::connect(this, &avimApp::raw_message_recieved, this, [this, target, chat_widget, group_data_model](std::string _target, std::string data)
		{
			// 检查 data 是不是发给这个群的
			// 就是检查发送人是不是这个 group 啦!
			if (target != _target)
				return;
			// TODO 解析 control 消息!
			std::string _sender;
			auto ctl_msg = decode_control_message(data, _sender);

			if (ctl_msg->GetTypeName() == "proto.group.list_response")
			{
				// 解析返回的群成员列表!
				auto list_response = dynamic_cast<proto::group::list_response*>(ctl_msg.get());

				if (list_response->result() == proto::group::list_response::OK)
				{
					m_members_of_group[_target]->clear();
					// 填充!
					for (auto i: list_response->list())
						m_members_of_group[_target]->push_back(i);
				}

				// 通知好了

				auto top = group_data_model->index(0,0);
				auto bottom = group_data_model->index(group_data_model->rowCount(),1);

				group_data_model->dataChanged(top, bottom);
				chat_widget->group_updated();
			}

		}, Qt::QueuedConnection);
	}

	QMetaObject::Connection slot_connect = QObject::connect(this, &avimApp::message_recieved, this, [this, target, chat_widget](std::string _target, im_message pkt)
	{
		if (target == _target && pkt.is_message)
			chat_widget->append_message(pkt.impkt);
	}, Qt::QueuedConnection);

	m_chats.insert(std::pair<std::string, QWidget*>(target, (QWidget*)chat_widget));

	connect(chat_widget, &avui::ChatWidget::windowclosed, this, [this, target, slot_connect]()
	{
		m_chats.erase(target);
		QObject::disconnect(slot_connect);
	});

	return chat_widget;
}

void avimApp::on_message_recieve(std::string target, im_message pkt)
{
	if (m_chats.find(target) == m_chats.end())
	{
		if (pkt.is_message)
			start_chat_with(target, pkt.is_group_message)->append_message(pkt.impkt);
	}
}

void avimApp::send_raw_message(std::string target, std::string msg)
{
	m_avkernel.async_sendto(target, msg, [](boost::system::error_code ec)
	{
		post_on_gui_thread([ec]()
		{
			// TODO 处理消息发送成功 OR 失败的结果
		});
	});
}

void avimApp::send_im_message(std::string target, message::message_packet pkt)
{
	m_avkernel.async_sendto(target, encode_im_message(pkt), [](boost::system::error_code ec)
	{
		post_on_gui_thread([ec]()
		{
			// TODO 处理消息发送成功 OR 失败的结果
		});
	});
}

void avimApp::send_group_message(std::string target, message::message_packet pkt)
{
	// TODO 获取本群的加密 key
	std::string my_group_key = "";

	// TODO 获取本群当前 key 是第几个 key

	uint32_t keyid = 0;

	m_avkernel.async_sendto(target, encode_group_message(m_self_addr, my_group_key, keyid, pkt), [](boost::system::error_code ec)
	{
		post_on_gui_thread([ec]()
		{
			// TODO 处理消息发送成功 OR 失败的结果
		});
	});

}

