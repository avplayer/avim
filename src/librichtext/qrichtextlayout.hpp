#pragma once

#include <vector>
#include <QWidget>
#include <QLayout>

class QRichTextLayout : public QLayout
{
	Q_OBJECT
public:
	explicit QRichTextLayout(QWidget *parent, int margin = -1);
	explicit QRichTextLayout(int margin = -1);

public:
	virtual QSize minimumSize() const;
	virtual QSize sizeHint() const;
	virtual void addItem(QLayoutItem*);
	virtual QLayoutItem* itemAt(int index) const;
	virtual QLayoutItem* takeAt(int index);
	virtual int count() const;

	virtual Qt::Orientations expandingDirections() const;
	virtual void setGeometry(const QRect&);

	virtual bool hasHeightForWidth() const {return true;}
	virtual int heightForWidth(int) const;

Q_SIGNALS:

public Q_SLOTS:


private:
	int doLayout(const QRect &rect, bool testOnly) const;

	std::vector<QLayoutItem*> m_item_list;
};

