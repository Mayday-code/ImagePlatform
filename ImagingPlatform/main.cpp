#include "ImagingPlatform.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImagingPlatform w;
    w.show();
	return a.exec();
}
