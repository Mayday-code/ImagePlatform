#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>
#include <QSerialPort>
#include <QGraphicsScene>
#include <atomic>
#include "ui_ImagingPlatform.h"
#include "ImageViewer.h"
#include "Previewer.h"
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
	void addFOV(const QPoint& point, const QImage& image, int r, int c);
	void showPreviewer();

private:
    Ui::ImagingPlatformClass *ui;
	QGraphicsScene *m_scene;
	ImageViewer* m_viewer;
	Previewer *m_previewer = nullptr;

	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<Stage> m_stage;

	std::atomic<bool> m_scanStop{ false };

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

	void on_moveTo(const QPoint& point);

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
	void on_addFOV(const QPoint& point, const QImage& pixmap, int r, int c);
	void on_showPreviewer();
};

