
#pragma once

#include <QObject>
#include <boost/concept_check.hpp>

void post_on_gui_thread(std::function<void()>);

class SyncObjec : QObject
{
	Q_OBJECT

	void do_post(std::function<void()> func);

private Q_SLOTS:
	void on_post(std::function<void()> qfunc_ptr){(qfunc_ptr)();}

Q_SIGNALS:
	void post(std::function<void()>);

public:
	SyncObjec()
	{
		qRegisterMetaType<std::function<void()>>("std::function<void()>");
		connect(this, SIGNAL(post(std::function<void()>)),
				this, SLOT(on_post(std::function<void()>)), Qt::QueuedConnection);
	}

	friend void post_on_gui_thread(std::function<void()>);

};
