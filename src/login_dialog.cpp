
#include <memory>
#include <QFileDialog>
#include <QDebug>

#include "login_dialog.hpp"
#include "ui_login_dialog.h"

login_dialog::login_dialog(avim::ini* _cfg)
	: QDialog(nullptr, Qt::WindowContextHelpButtonHint)
	, cfg(_cfg)
{
	m_ui.reset(new Ui::login_dialog());
	m_ui->setupUi(this);

	connect(m_ui->browse_cert_button, &QPushButton::clicked, [&]
	{
		auto filename = QFileDialog::getOpenFileName(this, tr("Open Certificate"));
		m_ui->cert_path->setText(filename);
	});
	connect(m_ui->browse_key_button, &QPushButton::clicked, [&]
	{
		auto filename = QFileDialog::getOpenFileName(this, tr("Open Key"));
		m_ui->key_path->setText(filename);
	});
	connect(m_ui->login_button, &QPushButton::clicked, this, &login_dialog::on_login);

	QObject::connect(m_ui->register_button, SIGNAL(clicked(bool)), this, SLOT(do_register_request()));

	m_ui->cert_path->setText(QString::fromStdString(cfg->get<std::string>("global.cert")));
	m_ui->key_path->setText(QString::fromStdString(cfg->get<std::string>("global.key")));
}

void login_dialog::do_register_request()
{
	m_register_dialog.reset(new register_dialog(this));
	QObject::connect(m_register_dialog.get(), &register_dialog::registe_user, this, &login_dialog::request_registering);
	if (m_register_dialog->exec())
	{
		// TODO 注册成功, 打开对话框要求保存证书
	}

}

void login_dialog::on_login()
{
	if (get_key_path() == "" || get_cert_path() == "")
	{
		qDebug() << "on_login() some thing is null!";
		return;
	}

	bool auto_lgoin = Qt::Checked == m_ui->login_automatically->checkState();
	if (Qt::Checked == m_ui->remember_me->checkState() || auto_lgoin)
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
	return m_ui->cert_path->text().toStdString();
}

std::string login_dialog::get_key_path()
{
	return m_ui->key_path->text().toStdString();
}

void login_dialog::closeEvent(QCloseEvent*e)
{
    QDialog::closeEvent(e);
	QApplication::quit();
}
