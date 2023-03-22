#pragma once

#include "Camera.h"
#include "TUCamApi.h"

class TUCam : public Camera
{
public:
	TUCam();
	~TUCam();

	virtual bool isSupportResolutionSwitching() const { return true; }

private:
	virtual bool init() override;
	virtual bool open() override;
	virtual bool startCapturing() override;
	virtual bool setDeviceExp(double exp_ms) override;
	virtual bool setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) override;

	void stopCap();
private:
	HDTUCAM m_handle = nullptr;
	TUCAM_INIT m_itApi;       // SDK API initialized object
	TUCAM_OPEN m_opCam;       // Open camera object
	TUCAM_FRAME m_frame;      // The frame object
};
