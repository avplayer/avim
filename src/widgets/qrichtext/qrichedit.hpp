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

#include "im.pb.h"

Q_DECLARE_METATYPE(message::message_packet)

class QRichEdit : public QTextEdit
{
	Q_OBJECT

	Q_PROPERTY(message::message_packet content READ get_content WRITE set_content STORED false)
	Q_PROPERTY(bool HeightForWidth READ hasHeightForWidth WRITE set_hasHeightForWidth )

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
    virtual QSize minimumSizeHint() const;

	virtual int heightForWidth(int) const;
public Q_SLOTS:
	void updateGeometry();

protected:
    double do_calc_line_length(const QString& text, const QFont&);

	void dropImage(const QUrl& url, const QImage& image);
	void dropGIF(const QUrl& url, QMovie* );

private:
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

	double m_natural_width, m_natural_height;

};
