#include "gavim.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	gavim w;
	w.show();
	return a.exec();
}
