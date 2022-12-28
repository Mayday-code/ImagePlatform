#include "DemoCam.h"

#include <iostream>
#include <random>

DemoCam::DemoCam()
{
	std::cout << "initializing DemoCam..." << std::endl;

	m_state = DeviceState::REGISTER;

	m_height = 600;
	m_width = 900;
	m_pixDepth = 1;
	m_channel = 1;

	m_img.resize(m_width, m_height, m_pixDepth, m_channel);

	m_cbuf.initialize(m_width, m_height, m_pixDepth, m_channel);

	std::cout << "camera is connected successfully" << std::endl;
}

void DemoCam::startSequenceAcquisition()
{
	if (m_state == DeviceState::NOTREGISTER) {
		std::cout << "ERROR : camara is not registered" << std::endl;
		return;
	}

	if (isCapturing()) {
		return;
	}

	std::lock_guard<std::mutex> lck(m_stopLock);
	m_stop = false;

	std::cout << "StartSequenceAcquisition..." << std::endl;

	std::thread thread_capture([this] {
		while (isCapturing()) {
			generateSyntheticImage();
			m_cbuf.insertImage(m_img.getPixels(), getImageWidth(), getImageHeight());
		}
	});

	thread_capture.detach();
}

void DemoCam::generateSyntheticImage()
{
	unsigned width = getImageWidth();
	unsigned height = getImageHeight();
	auto buf = const_cast<unsigned char*>(m_img.getPixels());

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