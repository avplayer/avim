#pragma once

#include <memory>
#include <QDialog>

namespace Ui {
    class register_dialog;
}

class register_dialog : public QDialog
{
    Q_OBJECT

public:
	register_dialog(QWidget*);

Q_SIGNALS:
	// 点击后发射
	void registe_user(QString user, QString mailaddr, QString phone);

public Q_SLOTS:
	void registe_done(bool is_success, QString reason= QString());

private Q_SLOTS:
	void register_button_clicked(bool);
private:
    std::shared_ptr<Ui::register_dialog> ui;
};
