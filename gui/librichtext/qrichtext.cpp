#include "qrichtext.hpp"

QRichText::QRichText(QWidget* parent)
	: QWidget(parent)
{
	qRegisterMetaType<QRichText*>("QRichText*");
}

void QRichText::wheelEvent(QWheelEvent*)
{
}

void QRichText::resizeEvent(QResizeEvent*)
{
}
