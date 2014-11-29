#include <QToolTip>
#include <QLabel>
#include <QTimer>
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

QRichText::~QRichText()
{
}

QRichText::QRichText(QWidget* parent)
	: QScrollArea(parent)
{
	qRegisterMetaType<QRichText*>("QRichText*");
	qRegisterMetaType<msg_block*>("msg_block*");

	m_container = new QWidget();

	setWidgetResizable(true);
	setWidget(m_container);

	setBackgroundRole(QPalette::Base);

	m_layout = new QRichTextLayout(m_container);

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
	QString htmlMsg;

	htmlMsg += QStringLiteral("<div><h>%1 说:</h>").arg(blk->sender.c_str());
	auto l = new QLabel(m_container);
	l->setWordWrap(true);
	l->setText(htmlMsg);
	m_layout->addWidget(l);
	l->show();

	for (message::avim_message im_message_item : blk->msg.avim())
	{
		if (im_message_item.has_item_text())
		{
			QString htmlMsg;
			message::text_message text_message = im_message_item.item_text();
			std::string text = text_message.text();
			// TODO 更好的格式化
			htmlMsg.append(QStringLiteral(" <p> %1</p>").arg(QString::fromStdString(text)));

			htmlMsg.append(QStringLiteral("<br /></div>"));
			auto l = new QLabel(m_container);
			l->setWordWrap(true);
			l->setText(htmlMsg);
			m_layout->addWidget(l);
			l->show();
		}
		// TODO 添加图片功能!
		// 图片来啦!
		else if (im_message_item.has_item_image())
		{
			QImage img;

			Q_ASSERT(img.loadFromData((const uchar*) im_message_item.item_image().image().data(), im_message_item.item_image().image().length()));

			auto l = new QLabel(m_container);
			l->setPixmap(QPixmap::fromImage(img));
//			l->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
			l->setFixedSize(QSize(img.width(), img.height()));
			m_layout->addWidget(l);
			l->setScaledContents(true);
			l->show();
		}
	}

	QTimer::singleShot(50, this,  SLOT(scroll_to_end()));
}

void QRichText::scroll_to_end()
{
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

