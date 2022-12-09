#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>
#include <QSerialPort>
#include <QGraphicsScene>
#include "ui_ImagingPlatform.h"
#include "PriorStage.h"
#include "TUCam.h"
#include "ImageViewer.h"

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
	void updateViewer();

private:
    Ui::ImagingPlatformClass *ui;
	QGraphicsScene *m_scene;
	ImageViewer* m_viewer;

	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<PriorStage> m_stage;

	QString savingDir = ".\\";

private:
	void init();

private slots:
	// camera slots
	void on_pushButton_live();

	// stage slots
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

	void on_updateViewer();
};

