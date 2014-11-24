
#include <functional>
#include "syncobj.hpp"

static SyncObjec syncobj;

void post_on_gui_thread(std::function<void()> func)
{
	syncobj.do_post(func);
}

void SyncObjec::do_post(std::function<void()> func)
{
    emit post(func);
}
