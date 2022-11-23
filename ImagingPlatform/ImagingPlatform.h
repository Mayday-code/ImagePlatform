#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ImagingPlatform.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImagingPlatformClass; };
QT_END_NAMESPACE

class ImagingPlatform : public QMainWindow
{
    Q_OBJECT

public:
    ImagingPlatform(QWidget *parent = nullptr);
    ~ImagingPlatform();

private:
    Ui::ImagingPlatformClass *ui;
};
