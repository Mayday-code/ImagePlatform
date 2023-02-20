#pragma once

#include "Camera.h"
#include <iostream>

class DemoCam : public Camera {
public:
	DemoCam();
	~DemoCam() { std::cout << "DemoCam destructed..." << std::endl; }

	void setExposure(double exp_ms) override { };
	void startSequenceAcquisition() override;

	// implement in base class?
	const unsigned char* getCircularBufferTop() override { return m_cbuf.getTopImageBuffer()->getPixels(); }
	unsigned long long getCircularBufferImageCount() const override { return m_cbuf.getImageCounter(); }

	bool save(const char* filename, int format) override {
		std::cout << "save finished" << std::endl;
		return true;
	}

private:
	void generateSyntheticImage();

private:
	ImgBuffer m_img;
};
