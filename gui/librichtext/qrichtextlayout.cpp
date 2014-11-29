
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
	return QSize(500, 300);
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

	int widthfor_text = effectiveRect.width() -  50;

	for (QLayoutItem* item : m_item_list)
	{
        QWidget *wid = item->widget();

		auto spaceY_inc = wid->heightForWidth(widthfor_text);

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


