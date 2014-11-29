#pragma once
#include <memory>
#include <list>
#include <QTextEdit>

class QRichEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit QRichEdit(QWidget*);
};
