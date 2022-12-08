#pragma once

#include "Camera.h"
#include "TUCamApi.h"

class TUCam : public Camera{
public:
	TUCam(int ID);
	~TUCam();

	virtual unsigned getImageWidth() const override { return m_width; }
	virtual unsigned getImageHeight() const override { return m_height; }

	virtual void setExposure(double exp_ms) override;
	virtual double getExposure() const override { return m_exp; }

	virtual void startSequenceAcquisition() override;
	virtual void stopSequenceAcquisition() override;
	virtual bool isCapturing() const override { 
		std::lock_guard<std::mutex> lck(m_stopLock);
		return !m_stop;
	}

	virtual const unsigned char* getCircularBufferTop() override { return m_cbuf.getTopImageBuffer().getPixels(); }
	virtual unsigned long long getCircularBufferImageCount() const override { return m_cbuf.getImageCounter(); }

private:
	TUCAMRET initApi();
	TUCAMRET unInitApi();
	TUCAMRET openCamera(unsigned uiIdx);

	TUCAMRET startCap();
	void stopCap();

private:
	HDTUCAM m_handle = nullptr;
	TUCAM_INIT m_itApi;       // SDK API initialized object
	TUCAM_OPEN m_opCam;       // Open camera object
	TUCAM_FRAME m_frame;      // The frame object
};
