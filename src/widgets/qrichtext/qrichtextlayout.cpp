
#include <QScrollArea>
#include <boost/concept_check.hpp>

#include "qrichtextlayout.hpp"

QRichTextLayout::QRichTextLayout(QWidget* parent, int margin)
	: QVBoxLayout(parent)
{
	setContentsMargins(margin, margin, margin, margin);
}

QRichTextLayout::QRichTextLayout(int margin)
{
	setContentsMargins(margin, margin, margin, margin);
}

QSize QRichTextLayout::sizeHint() const
{
	return minimumSize();
}

QSize QRichTextLayout::minimumSize() const
{
	int w = 0, h = 0;
	for (QLayoutItem* item : m_item_list)
	{
		if (item->isEmpty())
			continue;
        QWidget *wid = item->widget();

		QSize ws = wid->minimumSizeHint();

		if (w <= ws.width())
			w = ws.width();

	}
	return QSize(w, h);
}

int QRichTextLayout::heightForWidth(int width) const
{
	return doLayout(QRect(0, 0, width, 0), true);
}

Qt::Orientations QRichTextLayout::expandingDirections() const
{
	return 0;
}

void QRichTextLayout::addItem(QLayoutItem * item)
{
	m_item_list.push_back(item);
}

QLayoutItem* QRichTextLayout::itemAt(int index) const
{
	if ( index >=m_item_list.size())
		return nullptr;
	return m_item_list.at(index);
}

QLayoutItem* QRichTextLayout::takeAt(int index)
{
	QLayoutItem* item = m_item_list.at(index);
	m_item_list.erase(std::begin(m_item_list) + index);
	return item;
}

int QRichTextLayout::count() const
{
	return m_item_list.size();
}

void QRichTextLayout::setGeometry(const QRect& r)
{
	QLayout::setGeometry(r);
	doLayout(r, false);
}

/*
 * 这个就是真正的排版引擎了.
 */
int QRichTextLayout::doLayout(const QRect& rect, bool testOnly) const
{
	int left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);

	int spaceY = effectiveRect.top();
	int spaceX = effectiveRect.x();

	int widthfor_text = effectiveRect.width();

	for (QLayoutItem* item : m_item_list)
	{
		if (item->isEmpty())
			continue;
        QWidget *wid = item->widget();

		int spaceY_inc;

		if (wid->hasHeightForWidth())
			spaceY_inc = wid->heightForWidth(widthfor_text);
		else
			spaceY_inc = wid->sizeHint().height();
        if (!testOnly)
		{
            item->setGeometry(
				QRect(QPoint(0, spaceY), QSize(widthfor_text, spaceY_inc))
			);
		}
		spaceY += spaceY_inc;
	}
	return spaceY;
}


