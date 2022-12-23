#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>
#include <QSerialPort>
#include <QGraphicsScene>
#include "ui_ImagingPlatform.h"
#include "ImageViewer.h"
#include "Stage.h"
#include "Camera.h"

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
	void enableXYScan();

private:
    Ui::ImagingPlatformClass *ui;
	QGraphicsScene *m_scene;
	ImageViewer* m_viewer;

	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<Stage> m_stage;

private:
	void init();
	bool checkStage();

private slots:
	// camera slots
	void on_pushButton_live_clicked();

	// stage slots
	void on_pushButton_stageConnect_clicked();
	void on_pushButton_XLeftShift_clicked(); 
	void on_pushButton_YLeftShift_clicked();
	void on_pushButton_ZLeftShift_clicked();
	void on_pushButton_XRightShift_clicked();
	void on_pushButton_YRightShift_clicked();
	void on_pushButton_ZRightShift_clicked();
	void on_lineEdit_XSS_editingFinished();
	void on_lineEdit_YSS_editingFinished();
	void on_lineEdit_ZSS_editingFinished();
	void on_updateXYPosition();
	void on_updateZPosition(); 

	//viewer slot
	void on_updateViewer();

	//function slots
	void on_pushButton_XYScan_clicked();
	void on_enableXYScan();
};

