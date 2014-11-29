#include <QDebug>
#include <QBuffer>
#include <QFile>
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
		dropImage(url, qvariant_cast<QImage>(source->imageData()));
	}
	else if (source->hasUrls())
	{
		for (QUrl url : source->urls())
		{
			if (url.isLocalFile())
			{
				// 查询文件的 Mime 类型
				QMimeType type = m_minedb.mimeTypeForUrl(url);
				auto name = type.name();
				if (name == "image/png" || name == "image/jpeg" || name == "image/bmp")
				{
					QImage image;
					image.load(url.toLocalFile());
					dropImage(url, image);
				}

				if (name == "image/gif")
				{
					QUrl tmpurl(QString("dropped_image_%1").arg(m_dropped_image_tmp_idx++));

					QFile imgfile(url.toLocalFile());

					QByteArray ba;// = imgfile.readAll();

					QBuffer buffer(&ba);

					m_image_raw_data.insert(std::make_pair(tmpurl, ba));

					dropGIF(tmpurl, new QMovie(url.toLocalFile()));
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
	gif->setParent(this);

	m_gif[url].reset(gif);
	m_is_gif.append(url);

	gif->start();

	connect(gif, &QMovie::updated, this, [this, gif, url](const QRect&){

		QImage image = gif->currentImage();

		document()->addResource(QTextDocument::ImageResource, url, image);

		Q_EMIT textChanged();
//		update();

		viewport()->update();
	});

	connect(gif, &QMovie::started, this, [this, gif, url](){

		QImage image = gif->currentImage();

		document()->addResource(QTextDocument::ImageResource, url, image);
		auto textcursor = textCursor();
		textcursor.insertImage(url.toString());
		setTextCursor(textcursor);

		Q_EMIT textChanged();
	});

	QImage image = gif->currentImage();

	document()->addResource(QTextDocument::ImageResource, url, image);
	auto textcursor = textCursor();
	textcursor.insertImage(url.toString());
	setTextCursor(textcursor);


}

void QRichEdit::clear()
{
	QTextEdit::clear();
	m_is_gif.clear();
	m_gif.clear();
	m_image_raw_data.clear();
}
