#include <list>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <QMetaType>
#include <QTextBlock>
#include "app.hpp"

#ifdef _WIN32

#ifdef STATIC_QT5
#include <QtPlugin>
#include <windows.h>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

Q_IMPORT_PLUGIN(QICOPlugin);

#endif
#endif

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr);


static SyncObjec * _syncobj;

int main(int argc, char *argv[])
{
	SyncObjec syncobj;

	_syncobj = &syncobj;

	qRegisterMetaType<std::string>("std::string");
	// 初始化该初始化的东西
	OpenSSL_add_all_algorithms();
	// 创建 QApp 对象
	avimApp app(argc, argv);
	// 开跑
	return app.exec();
}


void post_on_gui_thread(std::function<void()> func)
{
	_syncobj->do_post(func);
}
