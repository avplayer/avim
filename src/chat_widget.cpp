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

#include "chat_widget.hpp"
#include "qrichtext.hpp"

namespace avui
{
	chat_widget::chat_widget(std::string chat_target, QWidget* parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		setWindowTitle(chat_target.c_str());
		m_chat_target = chat_target;
	}

	chat_widget::~chat_widget()
	{
		qDebug() << "~avim()";
	}

	void chat_widget::on_sendButton_clicked()
	{
		if (ui.messageTextEdit->toPlainText() == "")
		{
			qDebug() << "Can not send null!";
			return;
		}

		// TODO 从 GUI 控件里构造 protobuf 的聊天消息
		message_block msg_block;
		msg_block.sender = "me";
		msg_block.msg = get_message();
		msg_block.dir = QBoxLayout::LeftToRight;

		ui.messageBrowser->append_message(msg_block);

		ui.messageTextEdit->clear();
		ui.messageTextEdit->setFocus();

		Q_EMIT send_message(msg_block.msg);
	}

	void chat_widget::append_message(message::message_packet msgpkt)
	{
		message_block msg;
		msg.sender = m_chat_target;
		msg.msg = msgpkt;
		msg.dir = QBoxLayout::RightToLeft;

		ui.messageBrowser->append_message(msg);
	}

	message::message_packet chat_widget::get_message()
	{
		return ui.messageTextEdit->get_content();
	}

	void chat_widget::closeEvent(QCloseEvent* e)
	{
		Q_EMIT windowclosed();
		QWidget::closeEvent(e);
	}


} // namespace avui
