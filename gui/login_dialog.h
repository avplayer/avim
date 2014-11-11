#include <memory>
#include <QDialog>

#include "ui_login_dialog.h"

class login_dialog : public QDialog
{
	Q_OBJECT

public:
	login_dialog();

private:
	void on_login();

	std::unique_ptr<Ui::login_dialog> ui_;
};
