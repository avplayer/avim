#pragma once

#include <QWidget>
#include <QThread>
#include "ui_chat_widget.h"
#include "ini.h"
#include "login_dialog.h"

namespace avui
{

	class chat_widget : public QWidget
	{
		Q_OBJECT

	public:
		chat_widget(std::string title, QWidget* parent = 0);

		~chat_widget();

		QString getMessage();

	private:
		Ui::chatClass ui;

		//recvThread rv_thread_;
		std::string current_chat_target;

	private Q_SLOTS:

		void on_sendButton_clicked();

		void on_chatTarget_clicked();

		void recvHandle(const QString&, const QString&);
	};

} // namespace avim
