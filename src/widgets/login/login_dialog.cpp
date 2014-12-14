
#include <memory>
#include <QFileDialog>
#include <QDebug>

#include "login_dialog.hpp"
#include "ui_login_dialog.h"

login_dialog::login_dialog()
	: QDialog(nullptr, Qt::WindowContextHelpButtonHint)
{
	m_ui.reset(new Ui::login_dialog());
	m_ui->setupUi(this);

	connect(m_ui->cert_path, &QPushButton::clicked, [&]
	{
		auto filename = QFileDialog::getOpenFileName(this, tr("Open Certificate"));
		m_ui->cert_path->setText(filename);
	});
	connect(m_ui->key_path, &QPushButton::clicked, [&]
	{
		auto filename = QFileDialog::getOpenFileName(this, tr("Open Key"));
		m_ui->key_path->setText(filename);
	});
	connect(m_ui->login_button, &QPushButton::clicked, this, &login_dialog::on_login);

	QObject::connect(m_ui->register_button, SIGNAL(clicked(bool)), this, SLOT(do_register_request()));

	m_ui->cert_path->setText(cfg.value("global/cert").toString());
	m_ui->key_path->setText(cfg.value("global/key").toString());

#ifdef ANDROID
	showFullScreen();
#endif
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
		cfg.setValue("global/cert", get_cert_path());
		cfg.setValue("global/key", get_key_path());
		if (auto_lgoin)
		{
			cfg.setValue("global/auto_login", true);
		}
	}

	this->accept();
}

QString login_dialog::get_cert_path()
{
	return m_ui->cert_path->text();
}

QString login_dialog::get_key_path()
{
	return m_ui->key_path->text();
}

void login_dialog::closeEvent(QCloseEvent*e)
{
    QDialog::closeEvent(e);
	QApplication::quit();
}
