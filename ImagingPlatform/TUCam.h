#pragma once

#include "Camera.h"
#include "TUCamApi.h"

class TUCam : public Camera{
public:
	TUCam(int ID);
	~TUCam();

	virtual void setExposure(double exp_ms) override;

	virtual void startSequenceAcquisition() override;

	virtual const unsigned char* getCircularBufferTop() override { return m_cbuf.getTopImageBuffer()->getPixels(); }
	virtual unsigned long long getCircularBufferImageCount() const override { return m_cbuf.getImageCounter(); }

	bool save(const char* filename, int format) override;

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
	TUCAM_FILE_SAVE m_fs;	  // The file save object
};
