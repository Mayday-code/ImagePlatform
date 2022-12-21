#include "ImagingPlatform.h"
#include "DemoCam.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <algorithm>
#include <string>
#include <thread>
#include <iostream>

ImagingPlatform::ImagingPlatform(QWidget *parent): 
	QMainWindow(parent), 
	ui(new Ui::ImagingPlatformClass())
{
    ui->setupUi(this);

	//load settings
	QSettings settings(QApplication::applicationDirPath() + "/Configuration.ini", QSettings::IniFormat);
	QString savePath = settings.value("savePath", "").toString();
	if (!savePath.isEmpty()) {
		ui->lineEdit_savePath->setText(savePath);
	} else {
		ui->lineEdit_savePath->setText(QApplication::applicationDirPath());
	}

	qDebug("searching serial port...");
	for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
		qDebug() << "Name        : " << info.portName();
		qDebug() << "Description : " << info.description();
		qDebug() << "Manufacturer: " << info.manufacturer();
		//To do:整理格式
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
	//save settings
	QSettings settings(QApplication::applicationDirPath() + "/Configuration.ini", QSettings::IniFormat);
	settings.setValue("savePath", ui->lineEdit_savePath->text());

	delete ui;
}

void ImagingPlatform::init()
{
	connect(this, &ImagingPlatform::updateXYPosition, this, &ImagingPlatform::on_updateXYPosition);
	connect(this, &ImagingPlatform::updateZPosition, this, &ImagingPlatform::on_updateZPosition);

	connect(this, &ImagingPlatform::updateViewer, this, &ImagingPlatform::on_updateViewer);

	connect(ui->pushButton_chooseSavePath, &QPushButton::clicked, [this] {
		QString dir = QFileDialog::getExistingDirectory();
		this->ui->lineEdit_savePath->setText(dir);
	});
}

bool ImagingPlatform::checkStage()
{
	if (m_stage == nullptr || m_stage->getState() == DeviceState::NOTREGISTER) {
		QMessageBox::warning(this, "Warning", "Stage is Not Connected");
		return false;
	}

	return true;
}

void ImagingPlatform::trigger()
{
	triggerFinished = false;
	m_scanCond.notify_all();
	while (!triggerFinished);
}

void ImagingPlatform::on_pushButton_live_clicked()
{
	// if the camera is not registered, create it at first
	if (!m_camera) {
		//m_camera = std::make_unique<DemoCam>();
		m_camera = std::make_unique<TUCam>(0);
	}

	if (m_camera->state() == DeviceState::NOTREGISTER) {
		QMessageBox::warning(this, "Warning", "Camera is Not Registered");
		return;
	}

	if (m_camera->isCapturing()) {
		return;
	}

	m_camera->startSequenceAcquisition();

	qDebug() << "start living......";

	std::thread thread_living([this] {
		while (true) {
			if (m_camera->isCapturing()) {
				const uchar* data = m_camera->getCircularBufferTop();
				QImage image(data, m_camera->getImageWidth(), m_camera->getImageHeight(), QImage::Format_RGB888);
				m_viewer->setPixmap(QPixmap::fromImage(image.scaled(900, 600)));

				emit updateViewer();
			}
		}

	});
	thread_living.detach();
}

void ImagingPlatform::on_pushButton_stageConnect_clicked()
{
	m_stage = std::make_unique<PriorStage>();
	std::string currentText = ui->comboBox_stageSerial->currentText().toStdString();
	if (currentText.empty()) {
		QMessageBox::warning(this, "Warning", "Invalid Serial Port");
		return;
	}

	int portNum = atoi(currentText.substr(3, currentText.size()).c_str());
	m_stage->setPort(portNum);
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

	emit ui->lineEdit_XSS->editingFinished();//initialize the step size of stage
	emit ui->lineEdit_YSS->editingFinished();
	emit ui->lineEdit_ZSS->editingFinished();

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

//To do : 正则验证
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
	if (m_camera == nullptr || m_camera->state() == DeviceState::NOTREGISTER) {
		QMessageBox::warning(this, "Warning", "Camera is Not Registered");
		return;
	}

	if (!m_camera->isCapturing()) {
		QMessageBox::warning(this, "Warning", "Camera is Not Capturing");
		return;
	}

	int XStepNum = ui->spinBox_XStepNum->value();
	int YStepNum = ui->spinBox_YStepNum->value();

	if (XStepNum <= 0 || YStepNum <= 0) {
		QMessageBox::warning(this, "Warning", "Invalid Step Setting");
		return;
	}

	ui->pushButton_XYScan->setDisabled(true);

	std::thread thread_scan([this, XStepNum, YStepNum] {
		std::cout << "XY Scanning started..." << std::endl;
		std::cout << "XStepNum : " << XStepNum 
			<< " XStepSize : " << ui->lineEdit_XSS->text().toStdString() 
			<< std::endl;
		std::cout << "YStepNum : " << YStepNum 
			<< " YStepSize : " << ui->lineEdit_YSS->text().toStdString()
			<< std::endl;

		

		std::cout << "XY Scanning finished..." << std::endl;
		ui->pushButton_XYScan->setEnabled(false);
	});
	thread_scan.detach();
}
