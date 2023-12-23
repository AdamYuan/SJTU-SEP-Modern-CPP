#include "MainWindow.hpp"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	QFontDatabase::addApplicationFont(":/FontAwesome.otf");
	MainWindow w;
	w.show();
	return QApplication::exec();
}
