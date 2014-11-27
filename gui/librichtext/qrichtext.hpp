#pragma once

#include <QWidget>

class QRichText : public QWidget
{
	Q_OBJECT
public:
	QRichText(QWidget* parent);

public:

	// 给 avim 用的接口, 用于添加文字, 添加图片, etc etc 的显示


protected:
	virtual void wheelEvent(QWheelEvent*);
	virtual void resizeEvent(QResizeEvent*);

private:
};
