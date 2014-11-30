
#include <QObject>
#include <QTextObjectInterface>


enum { GIFTextFormat = QTextFormat::UserObject + 1 };

class QRichEdit;
class QGIFObjectInterface : public QObject, public QTextObjectInterface
{
Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
	QGIFObjectInterface(QRichEdit* parent)
		: m_parent(parent)
	{
	}
	virtual ~QGIFObjectInterface(){};

	int Property_Image_Name()
	{
		return QTextFormat::UserProperty + 1;
	}

private:
	virtual QSizeF intrinsicSize(QTextDocument* doc, int posInDocument, const QTextFormat& format);
	virtual void drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc, int posInDocument, const QTextFormat& format);
private:
	QRichEdit* m_parent;
};
