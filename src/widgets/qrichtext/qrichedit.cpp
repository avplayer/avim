#include <boost/algorithm/string.hpp>
#include <QDebug>
#include <QBuffer>
#include <QFile>
#include <QByteArray>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFrame>
#include <QTextImageFormat>
#include <QTextBlock>
#include <QTextLayout>
#include "qrichedit.hpp"
#include <avproto/message.hpp>

QRichEdit::QRichEdit(QWidget*parent)
	: QTextEdit(parent)
{
	m_dropped_image_tmp_idx = 0;
	m_hasHeightForWidth = QTextEdit::hasHeightForWidth();
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
					image.loadFromData(imgdata->get_bytes());
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

	QObject::connect(gif, &QMovie::updated, this, [this, gif, url](const QRect&)
	{
		QImage image = gif->currentImage();
		document()->addResource(QTextDocument::ImageResource, url, image);
		viewport()->update();
		Q_EMIT textChanged();
	});

	QImage image = gif->currentImage();

	document()->addResource(QTextDocument::ImageResource, url, image);
	auto textcursor = textCursor();
	textcursor.insertImage(url.toString());
	setTextCursor(textcursor);
}

const QByteArray& QRichEdit::get_image_data(const QString& name)
{
	auto meimg = m_image_raw_data.find(name);
	if (meimg != m_image_raw_data.end())
		return (meimg->second)->get_bytes();
	// 到 documet 的资源里找
	QVariant var = document()->resource(QTextDocument::ImageResource, name);
	QImage realimage = qvariant_cast<QImage>(var);

	// 格式化 realimage 数据到 QByteArray
	QByteArray ba;
	{ QBuffer io(&ba);
	io.open(QIODevice::WriteOnly);
	realimage.save(&io, "PNG");
	}
	auto imgdata = std::make_shared<image_data>(ba);
	auto inserted = m_image_raw_data.insert(std::make_pair(name, imgdata));

	return imgdata->get_bytes();
}

void QRichEdit::clear()
{
	for (auto &i : m_gif)
	{
		i.second->stop();
	}
	m_gif.clear();

	QTextEdit::clear();
	m_is_gif.clear();
	m_image_raw_data.clear();
}

message::message_packet QRichEdit::get_content()
{
	message::message_packet impkt;

	QTextDocument* doc = document();
	int blockcount = doc->blockCount();

	for (QTextBlock blk = doc->begin(); blk != doc->end(); blk = blk.next())
	{
		for (auto blkit = blk.begin(); blkit != blk.end(); ++blkit)
		{
			QTextFragment docfrag = blkit.fragment();
			auto txt = docfrag.text();
			if (docfrag.charFormat().isImageFormat())
			{
				QTextImageFormat imgformat = docfrag.charFormat().toImageFormat();

				const QByteArray& ba = get_image_data(imgformat.name());

				// nice, 弄到 impkt 里
				message::img_message item_content;

				item_content.set_animated(m_is_gif.contains(imgformat.name()));

				item_content.set_image(ba.data(), ba.length());

				impkt.mutable_avim()->Add()->mutable_item_image()->CopyFrom(item_content);
			}
			else
			{
				message::text_message item_content;

				for(QChar& qc : txt)
				{
					if (qc == QChar::LineSeparator)
					{
						qc = '\n';
					}
				}
				item_content.set_text(txt.toStdString());
				impkt.mutable_avim()->Add()->mutable_item_text()->CopyFrom(item_content);
			}
		}
	}
	return impkt;
}

void QRichEdit::set_content(message::message_packet msg)
{
	clear();

	// 记录每行的长度!
	std::vector<double> lines_width;

	double current_line_width = 0.0;

	for (message::avim_message im_message_item : msg.avim())
	{
		if (im_message_item.has_item_text())
		{
			message::text_message text_message = im_message_item.item_text();
			std::string text = text_message.text();

			QString qtext = QString::fromStdString(text);

			for (QChar& qc : qtext)
			{
				if (qc == '\n')
					qc = QChar::LineSeparator;
			}

			textCursor().insertText(qtext);
			QFont font = textCursor().charFormat().font();

			// 在这里计算 m_natural_width

			// 找换行符, 只计算到换行符号!
			auto newline_pos = text.find_first_of("\r\n");

			if (newline_pos == std::string::npos)
			{
				// 无换行, 那么就继续累加
				current_line_width += do_calc_line_length(QString::fromStdString(text), font);
			}
			else
			{
				if (newline_pos==0)
				{
					lines_width.push_back(current_line_width);
					current_line_width = 0;
				}

				std::vector<std::string> lines;
				boost::split(lines, text, boost::is_any_of("\r\n"));

				for(auto l : lines)
				{
					lines_width.push_back(current_line_width + do_calc_line_length(QString::fromStdString(l), font));
					current_line_width = 0;
				}
			}
		}
		else if (im_message_item.has_item_image())
		{
			message::img_message item_image = im_message_item.item_image();

			QUrl tmpurl(QString("image_%1").arg(m_dropped_image_tmp_idx++));
			auto img_data = std::make_shared<image_data>(
				QByteArray::fromRawData(item_image.image().data(),
					item_image.image().length()
				)
			);

			bool use_gif = false;

			if (im_message_item.item_image().has_animated())
			{
				use_gif = im_message_item.item_image().animated();
			}
			else
			{
				// 检查是 GIF 还是普通图片
				auto type = m_minedb.mimeTypeForData(img_data->get_bytes());
				use_gif = (type.name() == "image/gif");
			}

			img_data = std::make_shared<image_data>(
				QByteArray::fromRawData(item_image.image().data(),
					item_image.image().length()
				)
			);
			auto inserted = m_image_raw_data.insert(std::make_pair(tmpurl, img_data));
			if (use_gif)
			{
				auto qmovie = new QMovie(img_data->get_io_device());
				dropGIF(tmpurl, qmovie);
				current_line_width += qmovie->currentImage().width();
			}else
			{
				QImage img;
				img.loadFromData(img_data->get_bytes());
				dropImage(tmpurl, img);
				current_line_width +=  img.width();
			}
		}
	}

	lines_width.push_back(current_line_width);

	// 计算出最长的那条
	m_natural_width = *std::max_element(lines_width.begin(), lines_width.end());

	m_natural_height = lines_width.size() * 20;

	Q_EMIT textChanged();
}

bool QRichEdit::hasHeightForWidth() const
{
	bool is = QTextEdit::hasHeightForWidth();
	return m_hasHeightForWidth;
}

void QRichEdit::set_hasHeightForWidth(bool v)
{
	m_hasHeightForWidth = v;
}

int QRichEdit::heightForWidth(int w) const
{
	int h;
	if (m_hasHeightForWidth && w > 10)
	{
		std::unique_ptr<QTextDocument> doc(document()->clone());

		doc->size().setWidth(w);
		doc->setTextWidth(w-5);
		h = doc->size().height() + 5;
	}
	else
	{
		h = QWidget::heightForWidth(w);
	}
	return h;
}

QSize QRichEdit::sizeHint() const
{
	if (m_hasHeightForWidth)
	{
		// 来, 我们返回文本要求的最佳长度
		return QSize(m_natural_width + 18, heightForWidth(m_natural_width + 18));
	}
	return QAbstractScrollArea::sizeHint();
}

QSize QRichEdit::minimumSizeHint() const
{
    return QSize(1,1);
}

double QRichEdit::do_calc_line_length(const QString& text, const QFont& font)
{
	QTextLayout layout(text, font);
	layout.beginLayout();
	QTextLine  line = layout.createLine();
	if (line.isValid())
		line.setPosition(QPointF(0.0, 0.0));
	layout.endLayout();
	return layout.maximumWidth();
}

void QRichEdit::updateGeometry()
{
	QTextEdit::updateGeometry();
}
