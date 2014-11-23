#include <openssl/evp.h>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
namespace fs = boost::filesystem;

#include <QStandardPaths>
#include <QApplication>
#include "avim.h"
#include "login_dialog.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
	OpenSSL_add_all_algorithms();
	QApplication app(argc, argv);

	fs::path appdatadir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().toStdString();

	if (!fs::exists(appdatadir))
		fs::create_directories(appdatadir);

  avui::avim w;
  avim::ini cfg("config.ini");
  std::string auto_login = cfg.get<std::string>("global.auto_login");

  if (auto_login == "true") {
    if (w.init(cfg.get<std::string>("global.key"), cfg.get<std::string>("global.cert"))){
      w.show();
      return app.exec();
    }
    else {
      // TODO: 加进namespace avui
      login_dialog login;
      if (login.exec() == QDialog::Accepted)
      {
        if (!w.init(login.get_key_path(), login.get_cert_path())){
          return 1;
        }
        w.show();
        return app.exec();
      }
      else
        return 0;
    }
  }
  else {
    // TODO: 加进namespace avui
    login_dialog login;
    if (login.exec() == QDialog::Accepted)
    {
      if (!w.init(login.get_key_path(), login.get_cert_path())){
        return 1;
      }
      w.show();
      return app.exec();
    }
    else
      return 0;
  }
}
