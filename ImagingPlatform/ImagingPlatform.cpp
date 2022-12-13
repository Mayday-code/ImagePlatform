#include "ImagingPlatform.h"
#include "DemoCam.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <algorithm>
#include <string>
#include <thread>

using namespace std;

ImagingPlatform::ImagingPlatform(QWidget *parent): 
	QMainWindow(parent), 
	ui(new Ui::ImagingPlatformClass())
{
    ui->setupUi(this);

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
	delete ui;
}

void ImagingPlatform::init()
{
	connect(ui->pushButton_live, &QPushButton::clicked, this, &ImagingPlatform::on_pushButton_live);

	connect(ui->pushButton_stageConnect, &QPushButton::clicked, this, &ImagingPlatform::stageConnectClicked);
	connect(ui->pushButton_XLeftShift, &QPushButton::clicked, this, &ImagingPlatform::XLeftShiftClicked);
	connect(ui->pushButton_YLeftShift, &QPushButton::clicked, this, &ImagingPlatform::YLeftShiftClicked);
	connect(ui->pushButton_ZLeftShift, &QPushButton::clicked, this, &ImagingPlatform::ZLeftShiftClicked);
	connect(ui->pushButton_XRightShift, &QPushButton::clicked, this, &ImagingPlatform::XRightShiftClicked);
	connect(ui->pushButton_YRightShift, &QPushButton::clicked, this, &ImagingPlatform::YRightShiftClicked);
	connect(ui->pushButton_ZRightShift, &QPushButton::clicked, this, &ImagingPlatform::ZRightShiftClicked);
	connect(ui->lineEdit_XSS, &QLineEdit::editingFinished, this, &ImagingPlatform::XSSEditFinished);
	connect(ui->lineEdit_YSS, &QLineEdit::editingFinished, this, &ImagingPlatform::YSSEditFinished);
	connect(ui->lineEdit_ZSS, &QLineEdit::editingFinished, this, &ImagingPlatform::ZSSEditFinished);
	connect(this, &ImagingPlatform::updateXYPosition, this, &ImagingPlatform::on_updateXYPosition);
	connect(this, &ImagingPlatform::updateZPosition, this, &ImagingPlatform::on_updateZPosition);

	connect(this, &ImagingPlatform::updateViewer, this, &ImagingPlatform::on_updateViewer);
}

void ImagingPlatform::on_pushButton_live()
{
	// if the camera is not registered, create it at first
	if (!m_camera) {
		//m_camera = std::make_unique<DemoCam>();
		m_camera = std::make_unique<TUCam>(0);
	}

	if (m_camera->state() == CameraState::NOTREGISTER) {
		qDebug() << "ERROR : camera is not registered";
		return;
	}

	if (m_camera->isCapturing()) {
		return;
	}

	m_camera->startSequenceAcquisition();

	qDebug() << "start living......";

	std::thread thread_living([this] {
		while (true) {
			try {
				if (m_camera->isCapturing()) {
					const uchar* data = m_camera->getCircularBufferTop();
					QImage image(data, m_camera->getImageWidth(), m_camera->getImageHeight(), QImage::Format_RGB888);
					m_viewer->setPixmap(QPixmap::fromImage(image.scaled(900, 600)));

					emit updateViewer();
				}
			} catch (const std::exception& e) {
				qDebug() << e.what();
			}
		}

	});
	thread_living.detach();
}

void ImagingPlatform::stageConnectClicked()
{
	m_stage = std::make_unique<PriorStage>();
	string currentText = ui->comboBox_stageSerial->currentText().toStdString();
	int portNum = atoi(currentText.substr(3, currentText.size()).c_str());
	m_stage->setPort(portNum);
	m_stage->init();
	emit ui->lineEdit_XSS->editingFinished();//initialize the step size of stage
	emit ui->lineEdit_YSS->editingFinished();
	emit ui->lineEdit_ZSS->editingFinished();

	ui->pushButton_stageConnect->setStyleSheet("background-color: rgb(0,255,0)");
}

void ImagingPlatform::XLeftShiftClicked()
{
	//错误信息会在m_stage类内输出
	m_stage->mvrX(false);
	emit updateXYPosition();
}

void ImagingPlatform::YLeftShiftClicked() 
{
	m_stage->mvrY(false);
	emit updateXYPosition();
}

void ImagingPlatform::ZLeftShiftClicked()
{
	m_stage->mvrZ(false);
	emit updateZPosition();
}

void ImagingPlatform::XRightShiftClicked()
{
	m_stage->mvrX(true);
	emit updateXYPosition();
}

void ImagingPlatform::YRightShiftClicked()
{
	m_stage->mvrY(true);
	emit updateXYPosition();
}

void ImagingPlatform::ZRightShiftClicked()
{
	m_stage->mvrZ(true);
	emit updateZPosition();
}

//To do : 正则验证
void ImagingPlatform::XSSEditFinished()
{
	int SS = ui->lineEdit_XSS->text().toInt();
	m_stage->setXSS(SS);
}

void ImagingPlatform::YSSEditFinished()
{
	int SS = ui->lineEdit_YSS->text().toInt();
	m_stage->setYSS(SS);
}

void ImagingPlatform::ZSSEditFinished()
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

