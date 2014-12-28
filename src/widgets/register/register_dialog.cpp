#include "register_dialog.hpp"
#include "ui_register_dialog.h"

register_dialog::register_dialog(QWidget* parent)
	: QDialog(parent)
{
	ui.reset(new Ui::register_dialog);
	ui->setupUi(this);
 	connect(ui->button_registe, SIGNAL(clicked(bool)), this, SLOT(register_button_clicked(bool)));
}

void register_dialog::registe_done(bool is_success, QString reason)
{
	// 根据 done 不 done 的, 来 accept
	this->setEnabled(true);
	QDialog::done(is_success);
}

void register_dialog::register_button_clicked(bool)
{
	QString username =  ui->edit_username->text();
	QString email = ui->edit_email->text();
	QString phone = ui->edit_phone->text();

	Q_EMIT registe_user(username, email, phone);
	this->setEnabled(false);
}
