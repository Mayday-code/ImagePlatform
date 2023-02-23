#include "ImagingPlatform.h"
#include "DemoStage.h"
#include "PriorStage.h"
#include "TUCam.h"
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

using namespace std::literals;

ImagingPlatform::ImagingPlatform(QWidget *parent): 
	QMainWindow(parent), 
	ui(new Ui::ImagingPlatformClass())
{
    ui->setupUi(this);

	//load settings
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

	qDebug("searching serial port...");
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

	init();
}

ImagingPlatform::~ImagingPlatform()
{	
	m_running = false;
	if (m_camera && m_camera->isCapturing()) 
		m_camera->stopSequenceAcquisition();
	//save settings
	QSettings settings(QApplication::applicationDirPath() + "/Configuration.ini", QSettings::IniFormat);
	settings.setValue("savePath", ui->lineEdit_saveDir->text());
	settings.setValue("X step size", ui->lineEdit_XSS->text());
	settings.setValue("Y step size", ui->lineEdit_YSS->text());

	delete ui;
}

void ImagingPlatform::init()
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

bool ImagingPlatform::checkStage()
{
	if (m_stage == nullptr || m_stage->getState() == DeviceState::NOTREGISTER) {
		QMessageBox::warning(this, "Warning", "Stage is Not Connected");
		return false;
	}

	return true;
}

void ImagingPlatform::on_pushButton_live_clicked()
{
	// if the camera is not registered, create it at first
	if (!m_camera) {
		m_camera = std::make_unique<DemoCam>();
		//m_camera = std::make_unique<TUCam>(0);
	}

	if (m_camera->getState() == DeviceState::NOTREGISTER) {
		QMessageBox::warning(this, "Warning", "Camera is Not Registered");
		return;
	}

	if (m_camera->isCapturing()) {
		return;
	}

	m_camera->startSequenceAcquisition();

	qDebug() << "start living......";

	std::thread thread_living([this] {
		while (m_running) {
			while (m_camera->isCapturing()) {
				const uchar* data = m_camera->getCircularBufferTop();
				QImage image(data, m_camera->getImageWidth(), m_camera->getImageHeight(),
					m_camera->getChannel() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888);
				QPixmap pixmap = QPixmap::fromImage(image.scaled(900, 600));
				m_viewer->setPixmap(pixmap);

				emit updateViewer();
			}
		}
	});
	thread_living.detach();
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

void ImagingPlatform::on_updateViewer()
{
	m_viewer->update();
}

void ImagingPlatform::on_pushButton_XYScan_clicked()
{
	if (!checkStage()) return;
	if (m_camera == nullptr || m_camera->getState() == DeviceState::NOTREGISTER) {
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
		"µÈ´ýÊ±¼ä(ms)£º",
		150,
		0,
		1000,
		1,
		&bOk);
	if (!bOk) return;

	int XStepNum = ui->spinBox_XStepNum->value();
	int YStepNum = ui->spinBox_YStepNum->value();

	int saveFormat;
	switch (int index = ui->comboBox_format->currentIndex(); index) {
	case 0: saveFormat = TUFMT_PNG; break;
	case 1: saveFormat = TUFMT_TIF; break;
	case 2: saveFormat = TUFMT_JPG; break;
	case 3: saveFormat = TUFMT_RAW; break;
	}

	int initSerialNum = ui->lineEdit_serialNum->text().toInt();

	if (XStepNum <= 0 || YStepNum <= 0) {
		QMessageBox::warning(this, "Warning", "Invalid Step Setting");
		return;
	}

	ui->pushButton_XYScan->setDisabled(true);  

	bool save = !ui->radioButton_preview->isChecked();

	if (save) {
		//m_camera->stopSequenceAcquisition();

		QString QDir = this->ui->lineEdit_saveDir->text();
		m_camera->setSaveDir(QDir.toStdString().c_str());
	}
	else {
		Previewer* p = findChild<Previewer*>();
		if (p) p->close();
		m_previewer = new Previewer(this);
		connect(m_previewer, &Previewer::moveTo, this, &ImagingPlatform::on_moveTo);
	}

	std::thread thread_scan([this, XStepNum, YStepNum, save, saveFormat, initSerialNum, waitingTime] {
		std::cout << "XY Scanning started..." << std::endl;
		std::cout << "XStepNum : " << XStepNum 
			<< " XStepSize : " << ui->lineEdit_XSS->text().toStdString() 
			<< std::endl;
		std::cout << "YStepNum : " << YStepNum 
			<< " YStepSize : " << ui->lineEdit_YSS->text().toStdString()
			<< std::endl;

		int index = initSerialNum;
		char filename[256] = { 0 };

		bool forward = true;

		QString saveDir = ui->lineEdit_saveDir->text();
		sprintf_s(filename, 256, "%s\\%d.tif", saveDir.toStdString().c_str(), index);
		TinyTIFFWriterFile* tif = TinyTIFFWriter_open(filename, 8, TinyTIFFWriter_UInt,
			3,
			m_camera->getImageWidth(),
			m_camera->getImageHeight(),
			TinyTIFFWriter_RGB
		);

		emit setStopEnable(true);

		for (int y = 0; y < YStepNum && !m_scanStop; y++) {
			for (int x = 0; x < XStepNum && !m_scanStop; x++) {
				if (save) {
					const unsigned char* data = m_camera->getCircularBufferTop();
					TinyTIFFWriter_writeImage(tif, data);

					/*sprintf_s(filename, 100, "\\%04d", index);
					if (!m_camera->save(filename, saveFormat)) {
						std::cout << "ERROR : Unexpected stop when XY Scanning!" << std::endl;
						ui->pushButton_XYScan->setEnabled(true);
						return;
					}*/
				} else {
					const uchar* data = m_camera->getCircularBufferTop();
					QImage image(data, m_camera->getImageWidth(), m_camera->getImageHeight(),
						m_camera->getChannel() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888);
					auto pair = m_stage->getXYPos();
					QPoint point(pair.first, pair.second);
					emit addFOV(point, image, y, forward ? x : XStepNum - 1 - x);
				}

				index++;

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

		std::cout << "XY Scanning finished..." << std::endl;
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
	item->setPixmap(QPixmap::fromImage(image.scaled(450, 300)));
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