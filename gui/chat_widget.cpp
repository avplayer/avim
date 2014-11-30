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
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFrame>
#include <QTextImageFormat>
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

		ui.messageBrowser->append_message(msg);
	}

	message::message_packet chat_widget::get_message()
	{
		message::message_packet impkt;

		QTextDocument* doc = ui.messageTextEdit->document();
		int blockcount = doc->blockCount();

		for (QTextBlock blk = doc->begin(); blk != doc->end(); blk = blk.next())
		{
			for (auto blkit = blk.begin(); blkit != blk.end(); ++blkit)
			{
				QTextFragment docfrag = blkit.fragment();
				auto txt = docfrag.text();
				if (docfrag.charFormat().isImageFormat() )
				{
					QTextImageFormat imgformat = docfrag.charFormat().toImageFormat();

					const QByteArray& ba = ui.messageTextEdit->get_image_data(imgformat.name());

					// nice, 弄到 impkt 里
					message::img_message item_content;

					item_content.set_image(ba.data(), ba.length());

					impkt.mutable_avim()->Add()->mutable_item_image()->CopyFrom(item_content);
				}else
				{
					message::text_message item_content;
					item_content.set_text(txt.toStdString());

					impkt.mutable_avim()->Add()->mutable_item_text()->CopyFrom(item_content);
				}
			}
		}
		return impkt;
	}

	void chat_widget::closeEvent(QCloseEvent* e)
	{
		Q_EMIT windowclosed();
		QWidget::closeEvent(e);
	}


} // namespace avui
