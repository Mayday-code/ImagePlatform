#include "QtWidgetsApplication1.h"

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	uint8_t* data = new uint8_t[1024 * 1024];
	memset(data, 0, 1024 * 1024);
	QImage img = getQImage(data);
	//free(data);
	ui.label->setPixmap(QPixmap::fromImage(img));
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{}
