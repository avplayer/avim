#include <boost/bind.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QScrollBar>
#include <QStandardPaths>
#include <QInputDialog>

#include "chat.h"

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
    : QWidget(parent)
{
    ui.setupUi(this);
}

avim::~avim()
{
    qDebug() << "~avim()";
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
        //avcore_.sendto(current_chat_target, stdMsg);
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

} // namespace avui
