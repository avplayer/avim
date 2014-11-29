#pragma once

#include <QLayout>

class QRichTextLayout : public QLayout
{
	Q_OBJECT
public:
	explicit QRichTextLayout(QWidget *parent, int margin = -1);
	explicit QRichTextLayout(int margin = -1);

Q_SIGNALS:

public Q_SLOTS:

};

