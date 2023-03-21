#include "DemoCam.h"

#include <iostream>
#include <random>

DemoCam::DemoCam()
{
	if (!init()) return;
	if (!open()) return;

	m_state = CameraState::ONLINE;

	//初始设置要与界面的显示同步
	setROI(0, 0, 900, 600);
	setExposure(100);

	m_channel = 1;
	m_pixDepth = 1;

	m_img.resize(m_width, m_height, m_pixDepth, m_channel);

	m_cbuf.initialize(m_channel, m_width, m_height, m_pixDepth);

	std::cout << "相机连接成功" << std::endl;
}

bool DemoCam::startCapturing()
{
	std::lock_guard<std::mutex> lck(m_stateMutex);
	m_state = CameraState::LIVING;

	std::thread thread_capture([this] {
		while (isCapturing()) {
			generateSyntheticImage();
			m_cbuf.insertImage(m_img.getPixels(), getImageWidth(), getImageHeight());
		}

		std::cout << "退出采图" << std::endl;
	});

	thread_capture.detach();
}

void DemoCam::generateSyntheticImage()
{
	unsigned width = getImageWidth();
	unsigned height = getImageHeight();
	auto buf = m_img.getPixels();

	static std::default_random_engine generator;
	static std::uniform_int_distribution<int> distribution(0, 255);
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			try {
				long index = width * h + w;
				unsigned char val = static_cast<unsigned char>(distribution(generator));
				buf[index] = val;
			} catch (const std::exception& e) {
				std::cout << "EXCEPTION : " << e.what() << std::endl;
			}
		}
	}
}