
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
