#include "ImagingPlatform.h"
#include "DemoStage.h"
#include "PriorStage.h"
#include "TUCam.h"
#include "HamCam.h"
#include "DemoCam.h"
#include "PreviewItem.h"
#include "tinytiffwriter.h"

#include <QSerialPortInfo>
#include <QInputDialog>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <algorithm>
#include <string>
#include <thread>
#include <iostream>
#include <stdio.h>
#include <core.hpp>

using namespace std::literals;

ImagingPlatform::ImagingPlatform(QWidget *parent): 
	QMainWindow(parent), 
	ui(new Ui::ImagingPlatformClass())
{
    ui->setupUi(this);

	//加载上次退出时的一些设置
	QSettings settings(QApplication::applicationDirPath() + "/Configuration.ini", QSettings::IniFormat);
	QString savePath = settings.value("savePath", "").toString();
	QString XSS = settings.value("X step size", "600").toString();
	QString YSS = settings.value("Y step size", "350").toString();
	ui->lineEdit_XSS->setText(XSS);
	ui->lineEdit_YSS->setText(YSS);
	if (!savePath.isEmpty()) {
		ui->lineEdit_saveDir->setText(savePath);
	} else {
		ui->lineEdit_saveDir->setText(QApplication::applicationDirPath());
	}

	// 加载串口信息
	qDebug(u8"正在搜索串口...");
	for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
		qDebug() << "Name        : " << info.portName();
		qDebug() << "Description : " << info.description();
		qDebug() << "Manufacturer: " << info.manufacturer();
		ui->comboBox_stageSerial->addItem(info.portName());
	}

	m_scene = new QGraphicsScene();
	ui->FOV->setScene(m_scene);
	m_viewer = new ImageViewer();
	m_scene->addItem(m_viewer);

	initCameraComboBox();

	init();
}

ImagingPlatform::~ImagingPlatform()
{	
	m_running = false;
	if (m_camera && m_camera->isCapturing()) 
		m_camera->stopSequenceAcquisition();

	// 保存退出时用户的各个设置
	QSettings settings(QApplication::applicationDirPath() + "/Configuration.ini", QSettings::IniFormat);
	settings.setValue("savePath", ui->lineEdit_saveDir->text());
	settings.setValue("X step size", ui->lineEdit_XSS->text());
	settings.setValue("Y step size", ui->lineEdit_YSS->text());

	delete ui;
}

inline void ImagingPlatform::initCameraComboBox()
{
	// addItem()函数会发出currentIndexChanged(int index)这个信号，这个信号会绑定一个槽
	// 但这时我不想触发绑定的槽，因此先要把信号block掉
	ui->comboBox_camera->blockSignals(true);

	ui->comboBox_camera->addItem("Demo", "DemoCam");
	ui->comboBox_camera->addItem(u8"滨松", "HamCam");
	ui->comboBox_camera->addItem(u8"鑫图", "TUCam");

	// addItem()完成之后再取消block
	ui->comboBox_camera->blockSignals(false);
}

inline void ImagingPlatform::init()
{
	connect(this, &ImagingPlatform::updateXYPosition, this, &ImagingPlatform::on_updateXYPosition);
	connect(this, &ImagingPlatform::updateZPosition, this, &ImagingPlatform::on_updateZPosition);

	connect(this, &ImagingPlatform::updateViewer, this, &ImagingPlatform::on_updateViewer);

	connect(this, &ImagingPlatform::enableXYScan, this, &ImagingPlatform::on_enableXYScan);
	connect(this, &ImagingPlatform::addFOV, this, &ImagingPlatform::on_addFOV);
	connect(this, &ImagingPlatform::showPreviewer, this, &ImagingPlatform::on_showPreviewer);

	connect(ui->pushButton_chooseSaveDir, &QPushButton::clicked, [this] {
		QString dir;
		bool goon;
		do {
			dir = QFileDialog::getExistingDirectory();
			if (dir.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
				// Warn and repeat if dir contains Chinese character
				QMessageBox::warning(this, "Warning", "Cannot Contain Chinese Character");
				goon = true;
			} else {
				goon = false;
			}
		} while (goon);
			
		this->ui->lineEdit_saveDir->setText(dir);
	});
	connect(ui->pushButton_stopScan, &QPushButton::clicked, [this] { m_scanStop = true; });
	connect(this, &ImagingPlatform::setStopEnable, this, &ImagingPlatform::on_setStopEnable);
}

inline bool ImagingPlatform::checkStage()
{
	if (m_stage == nullptr || m_stage->getState() == StageState::OFFLINE) {
		QMessageBox::warning(this, "Warning", u8"位移台未连接");
		return false;
	}

	return true;
}

void ImagingPlatform::on_comboBox_camera_currentIndexChanged(int index)
{
	if (m_camera) {
		m_camera->stopSequenceAcquisition();
	}
	
	QString cameraName = ui->comboBox_camera->currentData().value<QString>();
	m_camera.reset();
	
	if (cameraName == "DemoCam") {
		m_camera = std::make_shared<DemoCam>();
	}
	else if (cameraName == "HamCam") {
		m_camera = std::make_shared<HamCam>();
	}
	else if (cameraName == "TUCam") {
		m_camera = std::make_shared<TUCam>();
	}
}

void ImagingPlatform::on_pushButton_live_clicked()
{
	if (!m_camera) {
		QMessageBox::warning(this, "warning", u8"未检测到相机");
		return;
	}

	if (m_camera->getState() == CameraState::OFFLINE) {
		QMessageBox::warning(this, "warning", u8"相机连接异常");
		return;
	}

	if (m_camera->isCapturing()) {
		return;
	}

	m_camera->startSequenceAcquisition();

	qDebug() << u8"开始实时显示";



	std::thread thread_living([this] {
		cv::Mat dest;
		while (m_running) {
			while (m_camera && m_camera->isCapturing()) {
				ImgBuffer* buffer = m_camera->getTopBuffer();
				if (buffer->channel() == 1) {
					if (buffer->depth() == 1) {
						dest = cv::Mat(buffer->height(), buffer->width(), CV_8UC1, buffer->getPixels());
					}
					else {
						cv::Mat origin(buffer->height(), buffer->width(), CV_16UC1, buffer->getPixels());
						// TODO:处理闪烁问题
						cv::normalize(origin, dest, 0., 255., cv::NORM_MINMAX, CV_8UC1);
					}
				}
				else {
					dest = cv::Mat(buffer->height(), buffer->width(), CV_8UC3, buffer->getPixels());
				}

				QImage image(dest.data, dest.cols, dest.rows,
					dest.channels() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888);

				double ratio = 1. * image.width() / image.height();
				QPixmap pixmap = QPixmap::fromImage(image.scaled(FOVHEIGHT * ratio, FOVHEIGHT));

				emit updateViewer(pixmap);
			}
		}
	});
	thread_living.detach();
}

void ImagingPlatform::on_pushButton_expApply_clicked()
{
	m_camera->setExposure(ui->doubleSpinBox_exp->value());
	// 设置曝光之后再将界面的曝光值改为相机的真实曝光值
	ui->doubleSpinBox_exp->setValue(m_camera->getExposure());
}


void ImagingPlatform::on_pushButton_roiApply_clicked()
{
	m_camera->setROI(ui->lineEdit_hPos->text().toInt(),
		ui->lineEdit_vPos->text().toInt(),
		ui->lineEdit_hSize->text().toInt(),
		ui->lineEdit_vSize->text().toInt());

	ui->lineEdit_hPos->setText(QString::number(m_camera->getHPos()));
	ui->lineEdit_vPos->setText(QString::number(m_camera->getVPos()));
	ui->lineEdit_hSize->setText(QString::number(m_camera->getImageWidth()));
	ui->lineEdit_vSize->setText(QString::number(m_camera->getImageHeight()));
}

void ImagingPlatform::on_lineEdit_hPos_editingFinished()
{
	int hPos = ui->lineEdit_hPos->text().toInt();
	ui->lineEdit_hPos->setText(QString::number(hPos / 4 * 4));
}

void ImagingPlatform::on_lineEdit_vPos_editingFinished()
{
	int vPos = ui->lineEdit_vPos->text().toInt();
	ui->lineEdit_vPos->setText(QString::number(vPos / 4 * 4));
}

void ImagingPlatform::on_lineEdit_hSize_editingFinished()
{
	int hSize = ui->lineEdit_hSize->text().toInt();
	ui->lineEdit_hSize->setText(QString::number(hSize / 4 * 4));
}

void ImagingPlatform::on_lineEdit_vSize_editingFinished()
{
	int vSize = ui->lineEdit_vSize->text().toInt();
	ui->lineEdit_vSize->setText(QString::number(vSize / 4 * 4));
}

void ImagingPlatform::on_pushButton_stageConnect_clicked()
{
	std::string currentText = ui->comboBox_stageSerial->currentText().toStdString();
	if (currentText.empty()) {
		//QMessageBox::warning(this, "Warning", "Invalid Serial Port");
		//return;
		m_stage = std::make_unique<DemoStage>();
	} else {
		int portNum = atoi(currentText.substr(3, currentText.size()).c_str());
		m_stage = std::make_unique<PriorStage>(portNum);
	}

	m_stage->init();

	ui->pushButton_XLeftShift->setEnabled(true);
	ui->pushButton_YLeftShift->setEnabled(true);
	ui->pushButton_ZLeftShift->setEnabled(true);
	ui->pushButton_XRightShift->setEnabled(true);
	ui->pushButton_YRightShift->setEnabled(true);
	ui->pushButton_ZRightShift->setEnabled(true);
	ui->lineEdit_XSS->setEnabled(true);
	ui->lineEdit_YSS->setEnabled(true);
	ui->lineEdit_ZSS->setEnabled(true);

	emit ui->lineEdit_XSS->editingFinished(); // initialize the step size of stage
	emit ui->lineEdit_YSS->editingFinished();
	emit ui->lineEdit_ZSS->editingFinished();

	emit updateXYPosition();
	emit updateZPosition();

	ui->pushButton_stageConnect->setStyleSheet("background-color: rgb(0,255,0)");
}

void ImagingPlatform::on_pushButton_XLeftShift_clicked()
{
	// error information will be printed by m_stage
	if (checkStage()) {
		m_stage->mvrX(false);
		emit updateXYPosition();
	}
}

void ImagingPlatform::on_pushButton_YLeftShift_clicked()
{
	if (checkStage()) {
		m_stage->mvrY(false);
		emit updateXYPosition();
	}
}

void ImagingPlatform::on_pushButton_ZLeftShift_clicked()
{
	if (checkStage()) {
		m_stage->mvrZ(false);
		emit updateZPosition();
	}
}

void ImagingPlatform::on_pushButton_XRightShift_clicked()
{
	if (checkStage()) {
		m_stage->mvrX(true);
		emit updateXYPosition();
	}
}

void ImagingPlatform::on_pushButton_YRightShift_clicked()
{
	if (checkStage()) {
		m_stage->mvrY(true);
		emit updateXYPosition();
	}
}

void ImagingPlatform::on_pushButton_ZRightShift_clicked()
{
	if (checkStage()) {
		m_stage->mvrZ(true);
		emit updateZPosition();
	}
}

void ImagingPlatform::on_moveTo(const QPoint& point)
{
	if (checkStage()) {
		m_stage->moveXY(point.x(), point.y());
		emit updateXYPosition();
	}
}

//To do : RegExp check
void ImagingPlatform::on_lineEdit_XSS_editingFinished()
{
	int SS = ui->lineEdit_XSS->text().toInt();
	m_stage->setXSS(SS);
}

void ImagingPlatform::on_lineEdit_YSS_editingFinished()
{
	int SS = ui->lineEdit_YSS->text().toInt();
	m_stage->setYSS(SS);
}

void ImagingPlatform::on_lineEdit_ZSS_editingFinished()
{
	int SS = ui->lineEdit_ZSS->text().toInt();
	m_stage->setZSS(SS);
}

void ImagingPlatform::on_updateXYPosition()
{
	auto XYPos = m_stage->getXYPos();
	ui->label_XPosition->setText(QString::number(XYPos.first));
	ui->label_YPosition->setText(QString::number(XYPos.second));
}

void ImagingPlatform::on_updateZPosition()
{
	double ZPos = m_stage->getZPos();
	ui->label_ZPosition->setText(QString::number(ZPos));
}

void ImagingPlatform::on_updateViewer(QPixmap pixmap)
{
	m_viewer->display(pixmap);
}

void ImagingPlatform::on_pushButton_XYScan_clicked()
{
	if (!checkStage()) return;
	if (m_camera == nullptr || m_camera->getState() == CameraState::OFFLINE) {
		QMessageBox::warning(this, "Warning", "Camera is Not Registered");
		return;
	}

	if (!m_camera->isCapturing()) {
		QMessageBox::warning(this, "Warning", "Camera is Not Capturing");
		return;
	}

	bool bOk = false;
	int waitingTime = QInputDialog::getInt(this,
		"Set waiting time",
		"等待时间(ms)：",
		150,
		0,
		1000,
		1,
		&bOk);
	if (!bOk) return;

	int XStepNum = ui->spinBox_XStepNum->value();
	int YStepNum = ui->spinBox_YStepNum->value();

	if (XStepNum <= 0 || YStepNum <= 0) {
		QMessageBox::warning(this, "Warning", "Invalid Step Setting");
		return;
	}

	ui->pushButton_XYScan->setDisabled(true);  

	bool save = !ui->radioButton_preview->isChecked();

	if (save) {
		//m_camera->stopSequenceAcquisition();

	}
	else {
		Previewer* p = findChild<Previewer*>();
		if (p) p->close();
		m_previewer = new Previewer(this);
		connect(m_previewer, &Previewer::moveTo, this, &ImagingPlatform::on_moveTo);
	}

	std::thread thread_scan([this, XStepNum, YStepNum, save, waitingTime] {
		std::cout << "XY Scanning started..." << std::endl;
		std::cout << "XStepNum : " << XStepNum 
			<< " XStepSize : " << ui->lineEdit_XSS->text().toStdString() 
			<< std::endl;
		std::cout << "YStepNum : " << YStepNum 
			<< " YStepSize : " << ui->lineEdit_YSS->text().toStdString()
			<< std::endl;

		bool forward = true;

		TinyTIFFWriterFile* tif = nullptr;

		// 一个stack最多只能存2GB，因此需要算出一个stack最多能存多少帧
		int maxFrame = 2048U * 1024U * 1024U / m_camera->getBitDepth() * m_camera->getChannel()
			* m_camera->getImageWidth() * m_camera->getImageHeight();
		int curFrame = maxFrame, subFile = 0;

		emit setStopEnable(true);

		cv::Mat dest;

		for (int y = 0; y < YStepNum && !m_scanStop; y++) {
			for (int x = 0; x < XStepNum && !m_scanStop; x++) {
				if (save) {
					ImgBuffer* buffer = m_camera->getTopBuffer();

					if (curFrame == maxFrame) {
						if (tif) {
							TinyTIFFWriter_close(tif);
						}
						char filename[256];
						sprintf_s(filename, sizeof filename, "%s\\%d_%d.tif", 
							ui->lineEdit_saveDir->text().toStdString().c_str(),
							fileIndex++, subFile++);
						tif = TinyTIFFWriter_open(filename,
							m_camera->getBitDepth() * 8,
							TinyTIFFWriter_UInt,
							m_camera->getChannel(),
							m_camera->getImageWidth(),
							m_camera->getImageHeight(),
							m_camera->getChannel() == 1 ? 
							TinyTIFFWriter_Greyscale : TinyTIFFWriter_RGB
						);
						curFrame = 0;
					}
					TinyTIFFWriter_writeImage(tif, buffer->getPixels());
					curFrame++;
				} else {
					ImgBuffer* buffer = m_camera->getTopBuffer();
					if (buffer->channel() == 1) {
						if (buffer->depth() == 1) {
							dest = cv::Mat(buffer->height(), buffer->width(), CV_8UC1, buffer->getPixels());
						}
						else {
							cv::Mat origin(buffer->height(), buffer->width(), CV_16UC1, buffer->getPixels());
							// TODO:处理闪烁问题
							cv::normalize(origin, dest, 0., 255., cv::NORM_MINMAX, CV_8UC1);
						}
					}
					else {
						dest = cv::Mat(buffer->height(), buffer->width(), CV_8UC3, buffer->getPixels());
					}

					double ratio = 1. * buffer->width() / buffer->height();
					auto pair = m_stage->getXYPos();
					QPoint point(pair.first, pair.second);
					emit addFOV(point, QImage(dest.data, dest.cols, dest.rows,
						dest.channels() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888).scaled(PREVIEWHEIGHT * ratio, PREVIEWHEIGHT), 
						y, forward ? x : XStepNum - 1 - x);
				}

				if (x != XStepNum - 1) {
					forward ? on_pushButton_XRightShift_clicked() :
						on_pushButton_XLeftShift_clicked();
					// directly delete?
					std::this_thread::sleep_for(std::chrono::milliseconds(waitingTime));
				}
			}

			if (y != YStepNum - 1) {// change direction
				on_pushButton_YRightShift_clicked();
				// directly delete?
				std::this_thread::sleep_for(std::chrono::milliseconds(waitingTime));
			}

			forward = !forward;
		}

		if (!m_scanStop) {
			TinyTIFFWriter_close(tif);
		}

		emit setStopEnable(false);
		m_scanStop = false;

		if (!save) emit showPreviewer();

		std::cout << "XY扫描结束" << std::endl;
		emit enableXYScan();

		m_camera->startSequenceAcquisition();
	});
	thread_scan.detach();
}

void ImagingPlatform::on_enableXYScan()
{
	ui->pushButton_XYScan->setEnabled(true);
}

void ImagingPlatform::on_addFOV(const QPoint& point, const QImage& image, int r, int c)
{
	PreviewItem* item = new PreviewItem(point);
	item->setPixmap(QPixmap::fromImage(image));
	m_previewer->addPreviewItem(item, r, c);
}

void ImagingPlatform::on_showPreviewer()
{
	m_previewer->show();
}

void ImagingPlatform::on_setStopEnable(bool flag)
{
	ui->pushButton_stopScan->setEnabled(flag);
}