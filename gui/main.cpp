#include <openssl/evp.h>

#include <QtWidgets/QApplication>
#include "gavim.h"

int main(int argc, char *argv[])
{
	OpenSSL_add_all_algorithms();
	QApplication a(argc, argv);
	gavim w;
	w.show();
	return a.exec();
}
