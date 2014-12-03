#pragma once

#include <QDialog>
#include "ui_register_dialog.h"

class register_dialog : public QDialog
{
    Q_OBJECT

public:
	register_dialog(QWidget*);

private:
    Ui::register_dialog ui;
};
