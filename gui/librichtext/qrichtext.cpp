#include <QToolTip>
#include <QLabel>
#include <boost/concept_check.hpp>
#include "qrichtextlayout.hpp"
#include "qrichtext.hpp"

class msg_block : public message_block
{
public:
	msg_block(message_block&&other)
		: message_block(other)
	{
	}
};

QRichText::QRichText(QWidget* parent)
	: QAbstractScrollArea(parent)
{
	qRegisterMetaType<QRichText*>("QRichText*");
	qRegisterMetaType<msg_block*>("msg_block*");

	this->setLayout(m_layout = new QRichTextLayout());

	connect(this, &QRichText::message_appended, this, &QRichText::on_message_append, Qt::QueuedConnection);
}

std::shared_ptr<message_block> QRichText::append_message(message_block textblock)
{
	auto blk = new msg_block(std::move(textblock));
	auto blk_ptr = std::shared_ptr<message_block>(blk,[](msg_block*blk){delete blk;});
	m_msgs.push_back(blk_ptr);

	Q_EMIT message_appended(blk);

	return blk_ptr;
}

void QRichText::on_message_append(msg_block* blk)
{
	QPoint pos;
	QToolTip::showText(pos, "stub, message appended", this);\

	QString htmlMsg;

	htmlMsg += QStringLiteral("<div><h>%1 说:</h>").arg(blk->sender.c_str());

	for (proto::avim_message im_message_item : blk->msg.avim())
	{
		if (im_message_item.has_item_text())
		{
			proto::text_message text_message = im_message_item.item_text();
			std::string text = text_message.text();
			// TODO 更好的格式化
			htmlMsg.append(QStringLiteral(" <p> %1</p>").arg(QString::fromStdString(text)));
		}
		// TODO 添加图片功能!
	}

	htmlMsg.append(QStringLiteral("<br /></div>"));
	auto l = new QLabel();
	l->setWordWrap(true);
	l->setText(htmlMsg);
	l->show();
	m_layout->addWidget(l);
}

