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

	void chat_widget::append_message(proto::avim_message_packet msgpkt)
	{
		message_block msg;
		msg.sender = m_chat_target;
		msg.msg = msgpkt;

		ui.messageBrowser->append_message(msg);
	}

	proto::avim_message_packet chat_widget::get_message()
	{
		// TODO
		proto::avim_message_packet impkt;

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

					QVariant img_res = doc->resource(QTextDocument::ImageResource, imgformat.name());

					QImage img = img_res.value<QImage>();

					QByteArray ba;
					QBuffer buffer(&ba);
					buffer.open(QIODevice::WriteOnly);
					img.save(&buffer, "JPEG"); // writes image into ba in JPEG format

					// nice, 弄到 impkt 里
					proto::img_message item_content;

					item_content.set_image(ba.data(), ba.length());

					impkt.mutable_avim()->Add()->mutable_item_image()->CopyFrom(item_content);
				}else
				{
					proto::text_message item_content;
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
