#pragma once
#include <memory>
#include <list>
#include <QTextEdit>
#include <QImage>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMovie>

class QRichEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit QRichEdit(QWidget*);
	~QRichEdit();

	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	virtual void insertFromMimeData(const QMimeData *source);

	void clear();

private:
	void dropImage(const QUrl& url, const QImage& image);
	void dropGIF(const QUrl& url, QMovie* );

	QMimeDatabase m_minedb;
	int m_dropped_image_tmp_idx;

	QList<QUrl> m_is_gif;
	std::map<QUrl, std::shared_ptr<QMovie>> m_gif;
	std::map<QUrl, QByteArray> m_image_raw_data;
};
