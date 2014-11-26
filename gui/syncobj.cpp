
#include <functional>
#include <QMetaType>
#include "syncobj.hpp"

static SyncObjec syncobj;

void post_on_gui_thread(std::function<void()> func)
{
	syncobj.do_post(func);
}

void SyncObjec::do_post(std::function<void()> func)
{
    Q_EMIT post(func);
}

SyncObjec::SyncObjec()
{
	qRegisterMetaType<std::function<void()>>("std::function<void()>");
	connect(this, &SyncObjec::post, this, &SyncObjec::on_post, Qt::QueuedConnection);
}
