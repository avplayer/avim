#include <openssl/evp.h>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
namespace fs = boost::filesystem;

#include <QStandardPaths>
#include <QApplication>
#include "gavim.h"
#include "login_dialog.h"

int main(int argc, char *argv[])
{
	OpenSSL_add_all_algorithms();
	QApplication app(argc, argv);

	fs::path appdatadir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();

	if (!fs::exists(appdatadir))
		fs::create_directories(appdatadir);

	gavim w;
	login_dialog login;
	if (login.exec() == QDialog::Accepted)
	{
		w.init(login.get_key_path(), login.get_cert_path());
		w.show();
		return app.exec();
	}
	else 
		return 0;
}
