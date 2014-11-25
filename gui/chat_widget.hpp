#pragma once

#include <QWidget>
#include "ui_chat_widget.h"

#include "avim_proto/im.pb.h"

Q_DECLARE_METATYPE(proto::avim_message_packet);

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
		// 编辑完成, 点击发送
		void send_message(proto::avim_message_packet);
		// 正编辑
		void editing_message();

	protected:
		QString getMessage();

	private:
		Ui::chatClass ui;

		//recvThread rv_thread_;
		std::string m_chat_target;

	private Q_SLOTS:

		void on_sendButton_clicked();
	};

} // namespace avim
