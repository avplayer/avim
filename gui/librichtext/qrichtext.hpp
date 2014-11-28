#pragma once
#include <memory>
#include <list>
#include <QWidget>

#include "avim_proto/im.pb.h"

// 这一个对象表示一条消息
struct message_block
{
	std::string sender, reciver;
	std::string time;

	proto::avim_message_packet msg;
};

class QRichText : public QWidget
{
	Q_OBJECT
public:
	QRichText(QWidget*);

Q_SIGNALS:
	// 滚动条拉动到顶部后还继续拉, 发出此信号
	// 接收到此信号就应该想办法从历史填充消息
	void overscroll_top(QRichText*);

public:

	// 给 avim 用的接口, 用于添加文字, 添加图片, etc etc 的显示


protected:
	virtual void wheelEvent(QWheelEvent*);
	virtual void resizeEvent(QResizeEvent*);

private:

	// 保存控件内部要显示的聊天消息
	std::list<std::shared_ptr<message_block>> m_msgs;
};
