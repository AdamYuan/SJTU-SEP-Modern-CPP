#include "MainWindow.h"

#include <QApplication>

#include "basic/Context.hpp"
#include "basic/Program.hpp"
#include "basic/Token.hpp"
#include <QFontDatabase>
#include <iostream>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	QFontDatabase::addApplicationFont(":/SourceCodePro-Regular.ttf");
	MainWindow w;
	w.show();
	return QApplication::exec();
}
