#pragma once

#include "Camera.h"
#include "dcamapi4.h"

class HamCam : public Camera
{
public:
	HamCam();
	~HamCam();

	virtual bool isSupportResolutionSwitching() const { return false; }
	virtual QList<QString> getResolution() { return {}; }

private:
	virtual bool init() override;
	virtual bool open() override;
	virtual bool startCapturing() override;
	virtual bool setDeviceExp(double exp_ms) override;
	virtual bool setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) override;

	void stopCap();

	virtual bool setDeviceRes(int index) {
		if (!isSupportResolutionSwitching()) {
			std::cout << "该相机不支持切换分辨率" << std::endl;
		}
		return false;
	}

	

private:
	HDCAM m_hdcam;
	HDCAMWAIT m_hwait;
	DCAMWAIT_START m_waitstart;
	DCAMBUF_FRAME m_bufframe;
	DCAMWAIT_OPEN m_waitopen;
	DCAMDEV_OPEN m_devopen;
};
