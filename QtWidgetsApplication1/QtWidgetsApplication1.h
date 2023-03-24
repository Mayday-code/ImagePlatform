#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <core.hpp>

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

	QImage getQImage(uint8_t* buffer) const {
		cv::Mat dest(1024, 1024, CV_8UC1, buffer);

		return QImage(dest.data, dest.cols, dest.rows, QImage::Format_Grayscale8);
	}

private:
    Ui::QtWidgetsApplication1Class ui;
};
