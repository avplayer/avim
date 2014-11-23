
#pragma once

#include <QObject>
#include <boost/concept_check.hpp>

class SyncObjec : QObject
{
	Q_OBJECT
private slots:
	void on_post(std::function<void()> qfunc_ptr){(qfunc_ptr)();}

signals:
	void post(std::function<void()>);

public:
	void do_post(std::function<void()> func)
	{
		emit post(func);
	}
	SyncObjec()
	{
		qRegisterMetaType<std::function<void()>>("std::function<void()>");
		connect(this, SIGNAL(post(std::function<void()>)),
				this, SLOT(on_post(std::function<void()>)), Qt::QueuedConnection);
	}
};
