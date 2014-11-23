#include <openssl/evp.h>

#include "app.hpp"

int main(int argc, char *argv[])
{
	// 初始化该初始化的东西
    OpenSSL_add_all_algorithms();
	// 创建 QApp 对象
    avimApp app(argc, argv);
	// 开跑
	return app.exec();
}
