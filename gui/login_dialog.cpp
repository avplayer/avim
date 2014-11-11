#include <memory>
#include "login_dialog.h"

#include <QFileDialog>

login_dialog::login_dialog()
	: ui_(std::make_unique<Ui::login_dialog>())
{
	ui_->setupUi(this);
	this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(ui_->browse_cert_button, &QPushButton::clicked, [&]
	{
		auto filename = QFileDialog::getOpenFileName(this, tr("Open Certificate"));
		ui_->cert_path->setText(filename);
	});
	connect(ui_->browse_key_button, &QPushButton::clicked, [&]
	{
		auto filename = QFileDialog::getOpenFileName(this, tr("Open Key"));
		ui_->key_path->setText(filename);
	});
	connect(ui_->login_button, &QPushButton::clicked, this, &login_dialog::on_login);
}

void login_dialog::on_login()
{
	// TODO: login verify and open main window if success
}

