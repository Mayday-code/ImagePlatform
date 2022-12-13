#pragma once

#include "Camera.h"

class DemoCam : public Camera {
public:
	DemoCam();

	void setExposure(double exp_ms) override { };
	void startSequenceAcquisition() override;

	// implement in base class?
	const unsigned char* getCircularBufferTop() override { return m_cbuf.getTopImageBuffer()->getPixels(); }
	unsigned long long getCircularBufferImageCount() const override { return m_cbuf.getImageCounter(); }

private:
	void generateSyntheticImage();

private:
	ImgBuffer m_img;
};
