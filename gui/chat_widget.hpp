#pragma once

#include <QWidget>
#include "ui_chat_widget.h"

namespace avui
{

	class chat_widget : public QWidget
	{
		Q_OBJECT

	public:
		chat_widget(std::string title, QWidget* parent = 0);

		~chat_widget();

	public:
	Q_SIGNALS:
		// 编辑完成, 点击发送
		void send_message();
		// 正编辑
		void editing_message();

	public:
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
