
#include "login_dialog.hpp"

#include <memory>
#include <QFileDialog>
#include <QtCore/QDebug>

login_dialog::login_dialog(avim::ini* _cfg)
	: ui_(std::make_unique<Ui::login_dialog>())
	, cfg(_cfg)
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

	QObject::connect(ui_->register_button, &QPushButton::clicked, [this](bool checked)
	{
		Q_EMIT register_new_user();
	});

	ui_->cert_path->setText(QString::fromStdString(cfg->get<std::string>("global.cert")));
	ui_->key_path->setText(QString::fromStdString(cfg->get<std::string>("global.key")));
}

void login_dialog::on_login()
{
	if (get_key_path() == "" || get_cert_path() == "")
	{
		qDebug() << "on_login() some thing is null!";
		return;
	}

	bool auto_lgoin = Qt::Checked == ui_->login_automatically->checkState();
	if (Qt::Checked == ui_->remember_me->checkState() || auto_lgoin)
	{
		cfg->put("global.cert", get_cert_path());
		cfg->put("global.key", get_key_path());
		if (auto_lgoin)
		{
			cfg->put("auto_login", "true");
		}
		cfg->write_to_file();
	}

	this->accept();
}

std::string login_dialog::get_cert_path()
{
	return ui_->cert_path->text().toStdString();
}

std::string login_dialog::get_key_path()
{
	return ui_->key_path->text().toStdString();
}
