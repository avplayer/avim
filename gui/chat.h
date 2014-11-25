#pragma once

#include <QWidget>
#include <QThread>
#include "ui_chat.h"
#include "ini.h"
#include "login_dialog.h"

namespace avui {

class avim : public QWidget
{
    Q_OBJECT

public:
    avim(QWidget *parent = 0);

    ~avim();

    QString getMessage();

private:
    Ui::chatClass ui;

	//recvThread rv_thread_;
    std::string current_chat_target;

private Q_SLOTS:

    void on_sendButton_clicked();

    void on_chatTarget_clicked();

    void recvHandle(const QString &, const QString &);
};

} // namespace avim
