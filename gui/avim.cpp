#include "avim.h"
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QScrollBar>
#include <QStandardPaths>
#include <QInputDialog>

#include <boost/bind.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

int pass_cb(char *buf, int size, int rwflag, char *u)
{
    int len;
    std::string tmp;
    /* We'd probably do something else if 'rwflag' is 1 */
    std::cout << "Enter pass phrase for " << u << " :";
    std::flush(std::cout);

    std::cin >> tmp;

    /* get pass phrase, length 'len' into 'tmp' */
    len = tmp.length();

    if (len <= 0) return 0;
    /* if too long, truncate */
    if (len > size) len = size;
    memcpy(buf, tmp.data(), len);
    return len;
}

namespace avui {

#if 0
recvThread::~recvThread()
{
    qDebug() << "~recvThread()";
}

void recvThread::run()
{
    qDebug() << "recv_thread::run()";

    OpenSSL_add_all_algorithms();
    boost::shared_ptr<BIO> keyfile(BIO_new_file(m_keyfile.c_str(), "r"), BIO_free);

    if (!keyfile)
	{
        std::cerr << "can not open " << m_keyfile << std::endl;
        ::exit(1);
    }

    boost::shared_ptr<RSA> rsa_key(
        PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, (pem_password_cb *) pass_cb, (void *) m_keyfile.c_str()),
        RSA_free
    );

    boost::shared_ptr<BIO> certfile(BIO_new_file(m_certfile.c_str(), "r"), BIO_free);
    if (!certfile)
	{
        std::cerr << "can not open " << m_certfile << std::endl;
        ::exit(1);
    }

    boost::shared_ptr<X509> x509_cert(
        PEM_read_bio_X509(certfile.get(), 0, 0, 0),
        X509_free
    );

    certfile.reset();
    keyfile.reset();

    // 连接到 im.avplayer.org:24950
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::shared_ptr<boost::asio::ip::tcp::socket> avserver(new boost::asio::ip::tcp::socket(io_service_));
    boost::asio::connect(*avserver, resolver.resolve(boost::asio::ip::tcp::resolver::query("avim.avplayer.org", "24950")));

    // 构造 avtcpif
    // 创建一个 tcp 的 avif 设备，然后添加进去
    boost::shared_ptr<avjackif> avinterface;
    avinterface.reset(new avjackif(avserver));
    avinterface->set_pki(rsa_key, x509_cert);
    avinterface->handshake();
    avcore_.add_interface(avinterface);
    std::string me_addr = av_address_to_string(*avinterface->if_address());
    // 添加路由表, metric越大，优先级越低
    avcore_.add_route(".+@.+", me_addr, avinterface->get_ifname(), 100);

    // 开协程异步接收消息
    boost::asio::spawn(io_service_, boost::bind(&recvThread::recv_msg, this, _1));
    io_service_.run();
}

void recvThread::recv_msg(boost::asio::yield_context yield_context)
{
    boost::system::error_code ec;
    std::string sender, data;
    for (; ;)
	{
        avcore_.async_recvfrom(sender, data, yield_context);
        emit recvReady(QString::fromStdString(sender), QString::fromStdString(data));
        qDebug() << "recv_msg()" << QString::fromStdString(data) << " from " << QString::fromStdString(sender);
    }
}
#endif

avim::avim(QWidget *parent)
    : QWidget(parent), avcore_(io_service_)
{
    ui.setupUi(this);
}

avim::~avim()
{
    qDebug() << "~avim()";
}

bool avim::init(const std::string &cur_key, const std::string &cur_cert)
{

}

QString avim::getMessage() {
    QString msg = ui.messageTextEdit->toPlainText();
    ui.messageTextEdit->clear();
    ui.messageTextEdit->setFocus();
    return msg;
}

void avim::on_sendButton_clicked()
{
    if (ui.messageTextEdit->toPlainText() == "")
	{
        qDebug() << "Can not send null!";
        return;
    }
    ui.messageBrowser->verticalScrollBar()->setValue(ui.messageBrowser->verticalScrollBar()->maximum());
    QString msg = getMessage();
    std::string stdMsg = msg.toStdString();
    QString htmlMsg = QString("<div>%1</div>").arg(msg);

    // TODO: new public method avim::loadLocalUserCssPreference
    // FIXME: memory leaking
    QTextDocument *doc = new QTextDocument(this);
    doc->setDefaultStyleSheet("div { color: red; text-align: right;}");
    // ui.messageBrowser->setDocument(doc);

    ui.messageBrowser->insertHtml(htmlMsg);
    qDebug() << "getMessage()" << msg;
    // 进入 IM 过程，发送一个 test  到 test2@avplayer.org
    boost::async([this, stdMsg]()
	{
        qDebug() << "on_sendButton_clicked()" << QString::fromStdString(current_chat_target) << " sendMsg: " << QString::fromStdString(stdMsg);
        avcore_.sendto(current_chat_target, stdMsg);
    });
}

/*
void avim::on_exitButton_clicked() {
  this->close();
}*/

void avim::on_chatTarget_clicked()
{
    bool ok;
    QString targetName = QInputDialog::getText(this, tr("Chat With Name"), tr("Input name:"), QLineEdit::Normal, ui.currentChat->text(), &ok);
    if (ok && !targetName.isEmpty()) {
        ui.currentChat->setText(targetName);
        current_chat_target = targetName.toStdString();
    }
}

void avim::recvHandle(const QString &sender, const QString &data)
{
    qDebug() << "recvHandle()" << sender << " sendMsg: " << data;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui.messageBrowser->setTextColor(Qt::blue);
    ui.messageBrowser->setCurrentFont(QFont("Times New Roman", 12));
    ui.messageBrowser->append("[" + sender + "]" + time);
    ui.messageBrowser->append(data);
}

void avim::closeEvent(QCloseEvent *)
{
    io_service_.stop();
}

} // namespace avui
