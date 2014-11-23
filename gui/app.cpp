
#include <boost/bind.hpp>

#include <QDebug>

#include "app.hpp"

void avimApp::on_post(std::function<void()> qfunc_ptr)
{
    (qfunc_ptr)();
}

avimApp::avimApp(int argc, char* argv[])
    : QApplication(argc, argv)
{
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    connect(this, SIGNAL(post(std::function<void()>)),
            this, SLOT(on_post(std::function<void()>)), Qt::QueuedConnection);
    // 开启 boost 线程跑 io_service
    m_io_thread = std::thread(boost::bind(&boost::asio::io_service::run, &m_io_service));

	// 发送一个测试 信号, 看能不能顺利的在2个线程里同步

	m_io_service.post(
		[this](){
			emit post([](){
				qDebug() << "test GUI thread and IO thread interactivity" ;
			});
		}
	);
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

    // 登录进去
    m_avim_client.reset(new avim_client(m_io_service, cur_key, cur_key));
    return true;
}
