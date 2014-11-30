#pragma once

#include <QFrame>

class QBubbleFrame : public QFrame
{
	Q_OBJECT
public:
	QBubbleFrame(QWidget*);
public:
	virtual int heightForWidth(int);
	virtual bool hasHeightForWidth();

protected:
	virtual void paintEvent(QPaintEvent*);

private:
};

