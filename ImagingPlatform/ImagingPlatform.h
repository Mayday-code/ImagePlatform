#pragma once

#include <QtWidgets/QMainWindow>
#include <QSerialPort>
#include "ui_ImagingPlatform.h"
#include "PriorStage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImagingPlatformClass; };
QT_END_NAMESPACE

class ImagingPlatform : public QMainWindow
{
    Q_OBJECT

public:
    ImagingPlatform(QWidget *parent = nullptr);
    ~ImagingPlatform();

signals:
	void updateXYPosition();
	void updateZPosition();

private:
    Ui::ImagingPlatformClass *ui;

	PriorStage* stage;

	QString savingDir = ".\\";

private:
	void init();

private slots:
	void stageConnectClicked();
	void XLeftShiftClicked();
	void YLeftShiftClicked();
	void ZLeftShiftClicked();
	void XRightShiftClicked();
	void YRightShiftClicked();
	void ZRightShiftClicked();
	void XSSEditFinished();
	void YSSEditFinished();
	void ZSSEditFinished();
	void on_updateXYPosition();
	void on_updateZPosition(); 
};
