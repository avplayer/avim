#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <memory>
#include <QDialog>

#include "ui_login_dialog.h"
#include "ini.h"

class login_dialog : public QDialog
{
	Q_OBJECT

public:
	login_dialog(avim::ini* _cfg);
	std::string get_cert_path();
	std::string get_key_path();

private:
	void on_login();
	std::unique_ptr<Ui::login_dialog> ui_;

	avim::ini* cfg;
};

#endif
