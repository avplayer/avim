
#include "login_dialog.h"
#include "ini.h"
#include "avim.h"

#include <memory>
#include <QFileDialog>
#include <QtCore/QDebug>

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
	
	avim::ini cfg("config.ini");	
	ui_->cert_path->setText(QString::fromStdString(cfg.get<std::string>("global.cert")));
	ui_->key_path->setText(QString::fromStdString(cfg.get<std::string>("global.key")));
}

void login_dialog::on_login()
{
	if ( get_key_path() == "" || get_cert_path() == "" )
	{
		qDebug() << "on_login() some thing is null!";
		return;
	}
  bool auto_lgoin = Qt::Checked == ui_->login_automatically->checkState();
	if (Qt::Checked == ui_->remember_me->checkState() || auto_lgoin)
	{
    // TODO: 把配置文件名定义成跨平台变量,而不依赖于执行目录当前路径
		avim::ini cfg("config.ini");
		cfg.put("global.cert", ui_->cert_path->text().toStdString());
		cfg.put("global.key", ui_->key_path->text().toStdString());
    if (auto_lgoin) {
      cfg.put("auto_login", "true");
    }
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
