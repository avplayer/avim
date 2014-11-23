
#include <functional>
#include "syncobj.hpp"

static SyncObjec syncobj;

void post_on_gui_thread(std::function<void()> func)
{
	syncobj.do_post(func);
}
