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
	std::lock_guard<std::mutex> lck(m_stopLock);
	m_stop = false;

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
	//int dice_roll = distribution(generator);  // generates number in the range 1..6
	for (int j = 0; j < height; j++) {
		for (int k = 0; k < width; k++) {
			try {
				long lIndex = width * j + k;
				/*
				val = 0;
				if (j == (k + 100)) {
					val = 1000;
				}
				*(pBuf + lIndex) = val;
				*/
				 unsigned char val = static_cast<unsigned char>(distribution(generator));
				 buf[lIndex] = val;
			} catch (const std::exception& e) {
				std::cout << "EXCEPTION : " << e.what() << std::endl;
			}
		}
	}
}