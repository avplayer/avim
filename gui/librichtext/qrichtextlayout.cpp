
#include "qrichtextlayout.hpp"

QRichTextLayout::QRichTextLayout(QWidget* parent, int margin)
	: QLayout(parent)
{
	setContentsMargins(margin, margin, margin, margin);
}

QRichTextLayout::QRichTextLayout(int margin)
{
	setContentsMargins(margin, margin, margin, margin);
}

QSize QRichTextLayout::sizeHint() const
{
	return QSize(500,300);
}

QSize QRichTextLayout::minimumSize() const
{
	return QSize(500,300);
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
	try
	{
		return m_item_list.at(index);
	}
	catch (const std::out_of_range&)
	{
		return nullptr;
	}
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

/*
 * 这个就是真正的排版引擎了.
 */
void QRichTextLayout::doLayout(const QRect& rect, bool testOnly) const
{
	int left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);

	for (QLayoutItem* item : m_item_list)
	{
        QWidget *wid = item->widget();
	}
}


