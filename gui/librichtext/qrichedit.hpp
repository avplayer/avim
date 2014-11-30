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
#include <QBuffer>

#include "avim_proto/im.pb.h"

class QGIFObjectInterface;
class QRichEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit QRichEdit(QWidget*);
	~QRichEdit();

	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	virtual void insertFromMimeData(const QMimeData *source);

	void clear();

	void set_content(message::message_packet);
	message::message_packet get_content();

public:
	virtual bool hasHeightForWidth() const;

	void set_hasHeightForWidth(bool = true);

	virtual QSize sizeHint() const;

	virtual int heightForWidth(int) const;
private:
	void dropImage(const QUrl& url, const QImage& image);
	void dropGIF(const QUrl& url, QMovie* );

	QMimeDatabase m_minedb;
	int m_dropped_image_tmp_idx;

	QList<QUrl> m_is_gif;

	struct image_data{
		QByteArray data;
		QBuffer io;

		image_data(const QByteArray& _data)
			: data(_data)
			, io(&data)
		{
			io.open(QIODevice::ReadOnly);
		}
		QIODevice* get_io_device()
		{
			return &io;
		}

		QByteArray& get_bytes()
		{
			return data;
		}
	};

	std::map<QUrl, std::shared_ptr<QMovie>> m_gif;
	std::map<QUrl, std::shared_ptr<image_data>> m_image_raw_data;
	const QByteArray& get_image_data(const QString&);

	bool m_hasHeightForWidth;

	QGIFObjectInterface* m_obj_interface_gif;
	friend class QGIFObjectInterface;
};
