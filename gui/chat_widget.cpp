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
#include <QTextBlock>
#include <QInputDialog>

#include "chat_widget.hpp"

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

	std::string chat_widget::getMessage()
	{
		QString msg = ui.messageTextEdit->toPlainText();
		return msg.toStdString();
	}

	void chat_widget::on_sendButton_clicked()
	{
		if (ui.messageTextEdit->toPlainText() == "")
		{
			qDebug() << "Can not send null!";
			return;
		}
		ui.messageBrowser->verticalScrollBar()->setValue(ui.messageBrowser->verticalScrollBar()->maximum());

		std::string msg = getMessage();
		QString htmlMsg = QString("<div><h>我 说:</h><p>%1</p><br /></div>").arg(msg.c_str());

		// TODO: new public method chat_widget::loadLocalUserCssPreference
		// FIXME: memory leaking
		//QTextDocument* doc = new QTextDocument(this);
		//doc->setDefaultStyleSheet("p { color: red; text-align: right;}");
		// ui.messageBrowser->setDocument(doc);

		ui.messageBrowser->insertHtml(htmlMsg);
		qDebug() << "getMessage()" << QString::fromStdString(msg);
		// 进入 IM 过程，发送一个 test  到 test2@avplayer.org

		// TODO 从 GUI 控件里构造 protobuf 的聊天消息

		qDebug() << "on_sendButton_clicked()" << QString::fromStdString(m_chat_target) << " sendMsg: " << QString::fromStdString(msg);
		Q_EMIT send_message(get_message());
		ui.messageTextEdit->clear();
		ui.messageTextEdit->setFocus();
	}

	void chat_widget::append_message(proto::avim_message_packet msgpkt)
	{
		qRegisterMetaType<QTextBlock>("QTextBlock");
		qRegisterMetaType<QTextCursor>("QTextCursor");

		QString htmlMsg;

		htmlMsg += QString("<div><h>%1 说:</h>").arg(m_chat_target.c_str());

		for (proto::avim_message im_message_item : msgpkt.avim())
		{
			if (im_message_item.has_item_text())
			{
				proto::text_message text_message = im_message_item.item_text();
				std::string text = text_message.text();
				// TODO 更好的格式化
				htmlMsg.append(QString(" <p> %1</p>").arg(QString::fromStdString(text)));
			}
			// TODO 添加图片功能!
		}

		htmlMsg.append(QString("<br /></div>"));

		ui.messageBrowser->insertHtml(htmlMsg);

		// TODO 从 protobuf 消息里解码出文字消息,图片消息 etc, 插入到 GUI 控件里
// 		qDebug() << "recvHandle()" << sender << " sendMsg: " << data;
// 		QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
// 		ui.messageBrowser->setTextColor(Qt::blue);
// 		ui.messageBrowser->setCurrentFont(QFont("Times New Roman", 12));
// 		ui.messageBrowser->append("[" + sender + "]" + time);
// 		ui.messageBrowser->append(data);
	}

	proto::avim_message_packet chat_widget::get_message()
	{
		// TODO
		proto::avim_message_packet ret;

		auto txtmsg = getMessage();

		proto::text_message txt_msg;
		txt_msg.set_text(txtmsg);

		ret.mutable_avim()->Add()->mutable_item_text()->CopyFrom(txt_msg);

		return ret;
	}

	void chat_widget::closeEvent(QCloseEvent* e)
	{
		Q_EMIT windowclosed();
		QWidget::closeEvent(e);
	}


} // namespace avui
