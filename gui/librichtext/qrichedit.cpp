#include "qrichedit.hpp"

QRichEdit::QRichEdit(QWidget*parent)
	: QTextEdit(parent)
{
}

QRichEdit::~QRichEdit()
{
}

bool QRichEdit::canInsertFromMimeData(const QMimeData *source) const
{
	return QTextEdit::canInsertFromMimeData(source);
}

void QRichEdit::insertFromMimeData(const QMimeData *source)
{
	QTextEdit::insertFromMimeData(source);
}
