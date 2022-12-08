#include "ImagingPlatform.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <algorithm>
#include <string>

using namespace std;

ImagingPlatform::ImagingPlatform(QWidget *parent): 
	QMainWindow(parent), 
	ui(new Ui::ImagingPlatformClass())
{
    ui->setupUi(this);

	qDebug("搜索串口...\n");
	for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
		qDebug() << "Name        : " << info.portName();
		qDebug() << "	Description : " << info.description();
		qDebug() << "	Manufacturer: " << info.manufacturer();
		qDebug() << endl;//To do:整理格式
		ui->comboBox_stageSerial->addItem(info.portName());
	}
}

ImagingPlatform::~ImagingPlatform()
{
	delete ui;

	stage->close();
	delete stage;
}

void ImagingPlatform::init()
{
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

}

void ImagingPlatform::stageConnectClicked()
{
	stage = new PriorStage();
	string currentText = ui->comboBox_stageSerial->currentText().toStdString();
	int portNum = atoi(currentText.substr(3, currentText.size()).c_str());
	stage->setPort(portNum);
	stage->init();
	ui->lineEdit_XSS->editingFinished();//初始化位移台步长
	ui->lineEdit_YSS->editingFinished();
	ui->lineEdit_ZSS->editingFinished();



	ui->pushButton_stageConnect->setStyleSheet("background-color: rgb(0,255,0)");
}

void ImagingPlatform::XLeftShiftClicked()
{
	//错误信息会在Stage类内输出
	stage->mvrX(false);
	emit updateXYPosition();
}

void ImagingPlatform::YLeftShiftClicked() 
{
	stage->mvrY(false);
	emit updateXYPosition();
}

void ImagingPlatform::ZLeftShiftClicked()
{
	stage->mvrZ(false);
	emit updateZPosition();
}

void ImagingPlatform::XRightShiftClicked()
{
	stage->mvrX(true);
	emit updateXYPosition();
}

void ImagingPlatform::YRightShiftClicked()
{
	stage->mvrY(true);
	emit updateXYPosition();
}

void ImagingPlatform::ZRightShiftClicked()
{
	stage->mvrZ(true);
	emit updateZPosition();
}

//To do : 正则验证
void ImagingPlatform::XSSEditFinished()
{
	int SS = ui->lineEdit_XSS->text().toInt();
	stage->setXSS(SS);
}

void ImagingPlatform::YSSEditFinished()
{
	int SS = ui->lineEdit_YSS->text().toInt();
	stage->setYSS(SS);
}

void ImagingPlatform::ZSSEditFinished()
{
	int SS = ui->lineEdit_ZSS->text().toInt();
	stage->setZSS(SS);
}

void ImagingPlatform::on_updateXYPosition()
{
	auto XYPos = stage->getXYPos();
	ui->label_XPosition->setText(QString::number(XYPos.first));
	ui->label_XPosition->setText(QString::number(XYPos.second));
}

void ImagingPlatform::on_updateZPosition()
{
	double ZPos = stage->getZPos();
	ui->label_ZPosition->setText(QString::number(ZPos));
}
