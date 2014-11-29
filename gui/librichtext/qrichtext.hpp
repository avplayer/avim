#pragma once
#include <memory>
#include <list>
#include <QScrollArea>
#include <QScrollBar>
#include <QLayout>
#include <boost/signals2.hpp>

#include "avim_proto/im.pb.h"

// 这一个对象表示一条消息
// 没办法, 继承自 QObject 是为了使用 信号/slot 机制
struct message_block
{
public:

	std::string sender, reciver;
	std::string time;

	proto::avim_message_packet msg;

	//boost::signals2::signal<void()> notify_change;
};

class msg_block;
class QRichTextLayout;
class QRichText : public QScrollArea
{
	Q_OBJECT
public:
	explicit QRichText(QWidget*);
	~QRichText();

Q_SIGNALS:
	// 滚动条拉动到顶部后还继续拉, 发出此信号
	// 接收到此信号就应该想办法从历史填充消息
	void overscroll_top(QRichText*);

	void message_appended(msg_block*);

private Q_SLOTS:
	void on_message_append(msg_block*);
	void scroll_to_end();

public:

	// 给 avim 用的接口, 用于添加文字, 添加图片, etc etc 的显示
	std::shared_ptr<message_block> append_message(message_block);
protected:
	QWidget* m_container;
	QRichTextLayout* m_layout;

private:
	// 保存控件内部要显示的聊天消息
	std::list<std::shared_ptr<message_block>> m_msgs;
};
