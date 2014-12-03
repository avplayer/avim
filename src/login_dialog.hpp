#pragma once

#include <memory>
#include <QDialog>

#include "register_dialog.hpp"
#include "ini.h"

namespace Ui{class login_dialog;}

class login_dialog : public QDialog
{
	Q_OBJECT

public:
	login_dialog(avim::ini* _cfg);
	std::string get_cert_path();
	std::string get_key_path();

Q_SIGNALS:
	void request_registring();

private:
	void on_login();
	std::shared_ptr<Ui::login_dialog> m_ui;
	std::shared_ptr<register_dialog> m_register_dialog;

	avim::ini* cfg;
};
