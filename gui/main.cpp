#include <list>
#include <vector>
#include <memory>
#include <random>
#include <QMetaType>
#include <QTextBlock>
#include "app.hpp"
#include "syncobj.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifdef _WIN32

#ifdef STATIC_QT5
#include <QtPlugin>
#include <windows.h>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

Q_IMPORT_PLUGIN(QICOPlugin);

#ifdef _DEBUG
#pragma comment(lib, "Qt5PlatformSupportd.lib")
#pragma comment(lib, "qwindowsd.lib")
#pragma comment(lib, "qicod.lib")
#else
#pragma comment(lib, "Qt5PlatformSupport.lib")
#pragma comment(lib, "qwindows.lib")
#pragma comment(lib, "qico.lib")
#endif

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "winmm.lib")

#endif
#endif

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr);


static SyncObjec * _syncobj;

int main(int argc, char *argv[])
{
	// work arround VC
	SyncObjec syncobj;
	_syncobj = &syncobj;

	qRegisterMetaType<std::string>("std::string");
	// 初始化该初始化的东西
	OpenSSL_add_all_algorithms();

#ifdef _WIN32
	// avoid calling RAND_poll that will crash on windows

	std::random_device rnd;
	for(int i=0; i<4; i++)
	{
		unsigned int seed = rnd();
		RAND_seed(&seed, sizeof(seed));
	}

#endif

	RAND_poll();

	// 创建 QApp 对象
	avimApp app(argc, argv);
	// 开跑
	return app.exec();
}


void post_on_gui_thread(std::function<void()> func)
{
	_syncobj->do_post(func);
}
