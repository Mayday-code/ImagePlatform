#pragma once

#include "Camera.h"
#include "dcamapi4.h"

class HamCam : public Camera
{
public:
	HamCam();
	~HamCam();

	virtual bool isSupportResolutionSwitching() const { return false; }

private:
	virtual bool init() override;
	virtual bool open() override;
	virtual bool startCapturing() override;
	virtual bool setDeviceExp(double exp_ms) override;
	virtual bool setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) override;

	void stopCap();
private:
	HDCAM m_hdcam;
	HDCAMWAIT m_hwait;
	DCAMWAIT_START m_waitstart;
	DCAMBUF_FRAME m_bufframe;
	DCAMWAIT_OPEN m_waitopen;
	DCAMDEV_OPEN m_devopen;
};
