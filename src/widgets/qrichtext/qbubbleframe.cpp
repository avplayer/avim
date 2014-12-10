#include "qbubbleframe.hpp"
#include <QPainter>
#include <QStylePainter>
#include <QPaintEvent>

QBubbleFrame::QBubbleFrame(QWidget* parent)
	: QFrame(parent)
{
}

int QBubbleFrame::heightForWidth(int w)
{
	QFrame::heightForWidth(w);
}

bool QBubbleFrame::hasHeightForWidth()
{
	return QFrame::hasHeightForWidth();
}

Qt::LayoutDirection QBubbleFrame::get_parent_layoutDirection()
{
	auto parent = parentWidget();
	if (!parent)
		return Qt::LeftToRight;
	auto d = parent->layoutDirection();

	if ( d = Qt::LayoutDirectionAuto)
	{
		return Qt::LeftToRight;
	}

	return d;
}

void QBubbleFrame::paintEvent(QPaintEvent*e)
{
	QPainter painter(this);

	// 绘制 圆角 ractangle
	Qt::LayoutDirection direction = get_parent_layoutDirection();

	QRect r = contentsRect();
	QRect rf;
	rf.setLeft(r.left());
	rf.setRight((double)r.right()-5.2);
	rf.setTop(r.top());
	rf.setBottom(r.bottom()-1);

	// 使用 8*8 的圆角

	painter.drawRoundedRect(rf, 11, 11);


	e->accept();
}

QSize QBubbleFrame::sizeHint() const
{
    return QFrame::sizeHint();
}

