#pragma once

#include <memory>
#include <QDialog>
#include <QSettings>

#include "register_dialog.hpp"

namespace Ui{class login_dialog;}

class login_dialog : public QDialog
{
	Q_OBJECT

public:
	login_dialog();
	QString get_cert_path();
	QString get_key_path();

Q_SIGNALS:
	void request_registering(QString user, QString mailaddr, QString phone);

private Q_SLOTS:
	void do_register_request();
	void on_login();
private:
    virtual void closeEvent(QCloseEvent*);
private:
	std::shared_ptr<Ui::login_dialog> m_ui;
	std::shared_ptr<register_dialog> m_register_dialog;

	QSettings cfg;
};
