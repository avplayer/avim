#pragma once
#include <memory>
#include <list>
#include <QTextEdit>

class QRichEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit QRichEdit(QWidget*);
	~QRichEdit();

	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	virtual void insertFromMimeData(const QMimeData *source);

};
