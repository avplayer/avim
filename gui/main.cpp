#include <openssl/evp.h>
#include "app.hpp"

#ifndef _WIN32
int main(int argc, char *argv[])
{
	// 初始化该初始化的东西
    OpenSSL_add_all_algorithms();
	// 创建 QApp 对象
    avimApp app(argc, argv);
	// 开跑
	return app.exec();
}
#else

#include <windows.h>

#ifdef QT_PLUGIN_STATIC_LINK

#include<QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

Q_IMPORT_PLUGIN(QICOPlugin);

#endif

int _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 初始化该初始化的东西
    OpenSSL_add_all_algorithms();
	// 创建 QApp 对象
    avimApp app(__argc, _targv);
	// 开跑
	return app.exec();
}
#endif
