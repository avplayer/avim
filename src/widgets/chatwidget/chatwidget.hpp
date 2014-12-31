#pragma once

#include <memory>
#include <QWidget>
#include <QAbstractListModel>

#include "im.pb.h"

class Ui_ChatWidget;
class ChatWidget : public QWidget
{
	Q_OBJECT

public:
	ChatWidget(std::string chat_target, QAbstractListModel* group_model = nullptr, QWidget* parent = nullptr);
	~ChatWidget();

public:
Q_SIGNALS:
	void windowclosed();
	// 编辑完成, 点击发送
	void send_message(message::message_packet);
	// 正编辑
	void editing_message();
public Q_SLOTS:
	void append_message(message::message_packet);

	void group_updated();

private Q_SLOTS:
	void on_sendButton_clicked();

protected:
	// 从控件里构建 avim_message_packet
	message::message_packet get_message();

	virtual void closeEvent(QCloseEvent*);

private:
	std::shared_ptr<Ui_ChatWidget> ui;

	//recvThread rv_thread_;
	std::string m_chat_target;


};
