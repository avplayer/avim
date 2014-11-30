#include <QDebug>
#include <QBuffer>
#include <QFile>
#include <QByteArray>
#include "qrichedit.hpp"

QRichEdit::QRichEdit(QWidget*parent)
	: QTextEdit(parent)
{
	m_dropped_image_tmp_idx = 0;
}

QRichEdit::~QRichEdit()
{
}

bool QRichEdit::canInsertFromMimeData(const QMimeData *source) const
{
	if (source->hasUrls())
	{
		// 只接受图片文件
		for (QUrl url : source->urls())
		{
			if (url.isLocalFile())
			{
				// 查询文件的 Mime 类型
				QMimeType type = m_minedb.mimeTypeForUrl(url);
				auto name = type.name();
				if (name == "image/png" || name == "image/jpeg" || name == "image/bmp" || name == "image/gif")
				{
					return true;
				}
			}
		}
	}
	return QTextEdit::canInsertFromMimeData(source);
}

void QRichEdit::insertFromMimeData(const QMimeData *source)
{
	if (source->hasImage())
	{
		QUrl url(QString("dropped_image_%1").arg(m_dropped_image_tmp_idx++));
		QByteArray ba;
		QImage img;
		{ QBuffer io(&ba);
		io.open(QIODevice::WriteOnly);
		img = qvariant_cast<QImage>(source->imageData());
		img.save(&io, "PNG");
		}
		auto imgdata = std::make_shared<image_data>(ba);
		auto inserted = m_image_raw_data.insert(std::make_pair(url, imgdata));
		dropImage(url, img);
	}
	else if (source->hasUrls())
	{
		for (QUrl url : source->urls())
		{
			if (url.isLocalFile())
			{
				QUrl tmpurl(QString("image_%1").arg(m_dropped_image_tmp_idx++));
				QFile imgfile(url.toLocalFile());

				if (!imgfile.open(QIODevice::ReadOnly))
					return;

				auto imgdata = std::make_shared<image_data>(imgfile.readAll());

				// 查询文件的 Mime 类型
				QMimeType type = m_minedb.mimeTypeForUrl(url);
				auto name = type.name();
				if (name == "image/png" || name == "image/jpeg" || name == "image/bmp")
				{
					auto inserted = m_image_raw_data.insert(std::make_pair(tmpurl, imgdata));
					QImage image;
					image.load(imgdata->get_bytes());
					dropImage(tmpurl, image);
				}
				else if (name == "image/gif")
				{
					auto inserted = m_image_raw_data.insert(std::make_pair(tmpurl, imgdata));

					dropGIF(tmpurl, new QMovie(inserted.first->second->get_io_device()));
				}
			}
		}
	}
	else
	{
		QTextEdit::insertFromMimeData(source);
	}
}

void QRichEdit::dropImage(const QUrl& url, const QImage& image)
{
	if (!image.isNull())
	{
		document()->addResource(QTextDocument::ImageResource, url, image);
		auto textcursor = textCursor();
		textcursor.insertImage(url.toString());
		setTextCursor(textcursor);

		Q_EMIT textChanged();
	}
}

void QRichEdit::dropGIF(const QUrl& url, QMovie* gif)
{
	m_gif[url].reset(gif);
	m_is_gif.append(url);

	gif->start();

	connect(gif, &QMovie::updated, this, [this, gif, url](const QRect&)
	{
		QImage image = gif->currentImage();
		document()->addResource(QTextDocument::ImageResource, url, image);
		viewport()->update();
	});

	QImage image = gif->currentImage();

	document()->addResource(QTextDocument::ImageResource, url, image);
	auto textcursor = textCursor();
	textcursor.insertImage(url.toString());
	setTextCursor(textcursor);
}

const QByteArray& QRichEdit::get_image_data(const QString& name)
{
	return m_image_raw_data[name]->get_bytes();
}

void QRichEdit::clear()
{
	QTextEdit::clear();
	m_is_gif.clear();
	m_gif.clear();
	m_image_raw_data.clear();
}

void QRichEdit::set_content(message::message_packet msg)
{
	for (message::avim_message im_message_item : msg.avim())
	{
		if (im_message_item.has_item_text())
		{
			message::text_message text_message = im_message_item.item_text();
			std::string text = text_message.text();
			textCursor().insertText(QString::fromStdString(text));
		}
		else if (im_message_item.has_item_image())
		{
			QUrl tmpurl(QString("image_%1").arg(m_dropped_image_tmp_idx++));
			QByteArray ba = QByteArray::fromRawData(im_message_item.item_image().image().data(),
				im_message_item.item_image().image().length());
			auto img_data = std::make_shared<image_data>(ba);
			auto inserted = m_image_raw_data.insert(std::make_pair(tmpurl, img_data));

			// 检查是 GIF 还是普通图片
			auto type = m_minedb.mimeTypeForData(ba);

			if (type.name() == "image/gif")
			{
				dropGIF(tmpurl, new QMovie(img_data->get_io_device()));
			}else{
				QImage img;
				img.loadFromData(img_data->get_bytes());
				dropImage(tmpurl, img);
			}
		}
	}
}
