#include <QToolTip>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <boost/concept_check.hpp>
#include "qrichtextlayout.hpp"
#include "qrichtext.hpp"
#include "qrichedit.hpp"

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
	l->show();
	m_layout->addWidget(l);

	// 把图片插入到 QRichEdit!
	auto richeditor = new QRichEdit(m_container);

	richeditor->setReadOnly(true);
	richeditor->set_hasHeightForWidth(true);
	richeditor->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

	richeditor->set_content(blk->msg);
	m_layout->addWidget(richeditor);

//	m_container->update();

	QTimer::singleShot(10, m_container,  SLOT(update()));
	QTimer::singleShot(0, this,  SLOT(scroll_to_end()));
}

void QRichText::scroll_to_end()
{
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

