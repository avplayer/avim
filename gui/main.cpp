#include <openssl/evp.h>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>
#include "gavim.h"

int main(int argc, char *argv[])
{
	OpenSSL_add_all_algorithms();
	QApplication a(argc, argv);

	fs::path appdatadir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();

	if(!fs::exists(appdatadir))
		fs::create_directories(appdatadir);

	gavim w;

	w.show();
	return a.exec();
}
