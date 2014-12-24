#include <boost/bind.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

#include <QDateTime>
#include <QDebug>
#include <QScrollBar>
#include <QStandardPaths>
#include <QBuffer>
#include <QImage>

#include "qrichtext.hpp"
#include "chatwidget.hpp"
#include "ui_chatwidget.h"

ChatWidget::ChatWidget(std::string chat_target,QAbstractListModel* group_model, QWidget* parent)
	: QWidget(parent)
{
	ui.reset(new Ui_ChatWidget);
	ui->setupUi(this);
	setWindowTitle(chat_target.c_str());
	m_chat_target = chat_target;
	if (group_model)
	{
		group_model->setParent(ui->listView);
		ui->listView->setModel(group_model);
	}
	else
	{
		ui->listView->hide();
		ui->listView->deleteLater();
		ui->listView = NULL;
	}

	QObject::connect(ui->messageTextEdit, SIGNAL(enterkey_pressed(bool)), ui->sendButton, SIGNAL(clicked(bool)));

}

ChatWidget::~ChatWidget()
{
	qDebug() << "~avim()";
}

void ChatWidget::on_sendButton_clicked()
{
	if (ui->messageTextEdit->toPlainText() == "")
	{
		qDebug() << "Can not send null!";
		return;
	}

	// TODO 从 GUI 控件里构造 protobuf 的聊天消息
	message_block msg_block;
	msg_block.sender = "me";
	msg_block.msg = get_message();
	msg_block.dir = QBoxLayout::RightToLeft;

	ui->messageBrowser->append_message(msg_block);

	ui->messageTextEdit->clear();
	ui->messageTextEdit->setFocus();

	Q_EMIT send_message(msg_block.msg);
}

void ChatWidget::append_message(message::message_packet msgpkt)
{
	message_block msg;
	if (msgpkt.has_sender())
		msg.sender = msgpkt.sender().username() + "@" + msgpkt.sender().domain();
	else
		msg.sender = m_chat_target;
	msg.msg = msgpkt;
	msg.dir = QBoxLayout::LeftToRight;

	ui->messageBrowser->append_message(msg);
}

void ChatWidget::group_updated()
{
	ui->listView->setEnabled(true);
}

message::message_packet ChatWidget::get_message()
{
	return ui->messageTextEdit->get_content();
}

void ChatWidget::closeEvent(QCloseEvent* e)
{
	Q_EMIT windowclosed();
	QWidget::closeEvent(e);
}

