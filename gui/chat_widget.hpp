#pragma once

#include <QWidget>
#include "ui_chat_widget.h"

#include "im.pb.h"


namespace avui
{
	class chat_widget : public QWidget
	{
		Q_OBJECT

	public:
		chat_widget(std::string chat_target, QWidget* parent = 0);
		~chat_widget();

	public:
	Q_SIGNALS:
		void windowclosed();
		// 编辑完成, 点击发送
		void send_message(message::message_packet);
		// 正编辑
		void editing_message();
	public Q_SLOTS:
		void append_message(message::message_packet);

	protected:
		// 从控件里构建 avim_message_packet
		message::message_packet get_message();

		virtual void closeEvent(QCloseEvent*);

	private:
		Ui::chatClass ui;

		//recvThread rv_thread_;
		std::string m_chat_target;

	private Q_SLOTS:

		void on_sendButton_clicked();
	};

} // namespace avim
