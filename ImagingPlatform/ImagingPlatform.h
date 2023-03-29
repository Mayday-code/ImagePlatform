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
	void updateViewer(QPixmap pixmap);
	void enableXYScan();
	void addFOV(const QPoint& point, const QImage& image, int r, int c);
	void showPreviewer();
	void setStopEnable(bool flag);

private:
    Ui::ImagingPlatformClass *ui;
	QGraphicsScene *m_scene;
	ImageViewer* m_viewer;
	Previewer *m_previewer = nullptr;

	std::shared_ptr<Camera> m_camera;
	std::shared_ptr<Stage> m_stage;

	std::atomic_bool m_scanStop{ false };
	std::atomic_bool m_running{ true };

	int fileIndex = 0;

private:
	// 将各个相机选项添加到UI
	void initCameraComboBox();

	// 关联信号和槽
	void init();
	bool checkStage();

private slots:
	// camera slots
	void on_comboBox_camera_currentIndexChanged(int index);
	void on_comboBox_res_currentIndexChanged(int index);
	void on_pushButton_live_clicked();
	void on_pushButton_expApply_clicked();
	void on_pushButton_roiApply_clicked();
	void on_lineEdit_hPos_editingFinished();
	void on_lineEdit_vPos_editingFinished();
	void on_lineEdit_hSize_editingFinished();
	void on_lineEdit_vSize_editingFinished();

	// stage slots
	void on_pushButton_stageConnect_clicked();
	void on_pushButton_XLeftShift_clicked(); 
	void on_pushButton_YLeftShift_clicked();
	void on_pushButton_ZLeftShift_clicked();
	void on_pushButton_XRightShift_clicked();
	void on_pushButton_YRightShift_clicked();
	void on_pushButton_ZRightShift_clicked();
	void on_pushButton_addAnchor_clicked();
	void on_pushButton_clearAnchor_clicked();
	void on_comboBox_anchor_activated(int index);

	void on_moveTo(const QPoint& point);

	void on_lineEdit_XSS_editingFinished();
	void on_lineEdit_YSS_editingFinished();
	void on_lineEdit_ZSS_editingFinished();
	void on_updateXYPosition();
	void on_updateZPosition(); 

	//viewer slot
	void on_updateViewer(QPixmap pixmap);

	//function slots
	void on_pushButton_XYScan_clicked();
	void on_enableXYScan();
	void on_addFOV(const QPoint& point, const QImage& pixmap, int r, int c);
	void on_showPreviewer();
	void on_setStopEnable(bool flag);
};

