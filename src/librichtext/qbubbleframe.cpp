#include "qbubbleframe.hpp"

QBubbleFrame::QBubbleFrame(QWidget* parent)
	: QFrame(parent)
{

}

int QBubbleFrame::heightForWidth(int w)
{
	return QFrame::heightForWidth(w);
}

bool QBubbleFrame::hasHeightForWidth()
{
	return QFrame::hasHeightForWidth();
}

void QBubbleFrame::paintEvent(QPaintEvent*e)
{
	QFrame::paintEvent(e);
}

