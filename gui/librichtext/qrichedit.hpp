#pragma once
#include <memory>
#include <list>
#include <QTextEdit>
#include <QImage>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>

class QRichEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit QRichEdit(QWidget*);
	~QRichEdit();

	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	virtual void insertFromMimeData(const QMimeData *source);

private:
	void dropImage(const QUrl& url, const QImage& image);
};
