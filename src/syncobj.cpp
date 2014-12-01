
#include <functional>
#include <QMetaType>
#include "syncobj.hpp"

void SyncObjec::do_post(std::function<void()> func)
{
    Q_EMIT post(func);
}

SyncObjec::SyncObjec()
{
	qRegisterMetaType<std::function<void()>>("std::function<void()>");
	connect(this, &SyncObjec::post, this, &SyncObjec::on_post, Qt::QueuedConnection);
}
