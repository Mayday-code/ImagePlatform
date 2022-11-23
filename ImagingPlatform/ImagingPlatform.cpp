#include "ImagingPlatform.h"

ImagingPlatform::ImagingPlatform(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImagingPlatformClass())
{
    ui->setupUi(this);
}

ImagingPlatform::~ImagingPlatform()
{
    delete ui;
}
