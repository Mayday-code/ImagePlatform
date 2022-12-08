#include "TUCam.h"

#include <QMessageBox>
#include <iostream>
#include <chrono>
#include <thread>

TUCam::TUCam(int ID)
{
	if (initApi() == TUCAMRET_NO_CAMERA) {
		std::cout << "ERROR : TUCAMRET_NO_CAMERA   please check camera connection!" << std::endl;
		return;
	}

	if (openCamera(ID) != TUCAMRET_SUCCESS) {
		std::cout << "ERROR : in open camera!" << std::endl;
		return;
	}

	m_handle = m_opCam.hIdxTUCam;
	m_state = CameraState::REGISTER;

	m_height = 5472;
	m_width = 3427;
	m_pixDepth = 1;
	m_channel = 3;

	m_cbuf.initialize(m_width, m_height, m_pixDepth, m_channel);

	std::cout << "camera is connected successfully" << std::endl;
}

TUCam::~TUCam()
{
	if (nullptr != m_handle) {
		TUCAM_Dev_Close(m_handle);
	}
	printf("Close the camera success\n");

	unInitApi();
}

void TUCam::setExposure(double exp_ms) 
{ 
	if (m_state == CameraState::NOTREGISTER) {
		std::cout << "ERROR : camera is not registered" << std::endl;
	}

	// if the camera is capturing when setting exposure, remember to restore it after that
	bool isCapturing = this->isCapturing();

	stopSequenceAcquisition();

	TUCAMRET ret = TUCAM_Prop_SetValue(m_handle, TUIDP_EXPOSURETM, exp_ms); 
	if (ret != TUCAMRET_SUCCESS) {
		std::cout << "ERROR : in setExposure";
	}
	std::cout << "camera setExposure:  " << exp_ms << std::endl;

	if (isCapturing) {
		startSequenceAcquisition();
	}
}

void TUCam::startSequenceAcquisition()
{
	std::cout << "StartSequenceAcquisition..." << 1 << std::endl;

	if (m_state == CameraState::NOTREGISTER) {
		std::cout << "StartSequenceAcquisition..." << 2 << std::endl;
		return;
	}

	std::lock_guard<std::mutex> g(m_stopLock);
	m_stop = false;

	std::cout << "StartSequenceAcquisition..." << 3 << std::endl;
	std::thread thread_capture([this] {
		auto ret = startCap();
		if (ret != TUCAMRET_SUCCESS) {
			std::cout << "ERROR : in sequence acquisition ( CODE = " << ret << " )" << std::endl;
			return;
		}

		while (isCapturing()) {
			if (TUCAMRET_SUCCESS == TUCAM_Buf_WaitForFrame(m_opCam.hIdxTUCam, &m_frame)) {
				m_cbuf.insertImage(m_frame.pBuffer + m_frame.usOffset, m_frame.usWidth, m_frame.usHeight);
			} else {
				std::cout << "ERROR : fail to grab the frame" << std::endl;
			}
		}

		stopCap();
		std::cout << "sequence acquisition exits" << std::endl;
	});
	thread_capture.detach();
}

void TUCam::stopSequenceAcquisition()
{
	std::unique_lock<std::mutex> g(m_stopLock);
	m_stop = true;
	g.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "StopSequenceAcquisition..." << std::endl;
}

TUCAMRET TUCam::initApi()
{
	/* Get the current directory */
	m_itApi.uiCamCount = 0;
	char configPath[] = "./";
	m_itApi.pstrConfigPath = configPath;

	TUCAM_Api_Init(&m_itApi);

	printf("Connect %d camera\n", m_itApi.uiCamCount);

	if (0 == m_itApi.uiCamCount)
	{
		return TUCAMRET_NO_CAMERA;
	}

	return TUCAMRET_SUCCESS;
}

TUCAMRET TUCam::unInitApi()
{
	return TUCAM_Api_Uninit();
}

TUCAMRET TUCam::openCamera(unsigned uiIdx)
{
	if (uiIdx >= m_itApi.uiCamCount)
	{
		return TUCAMRET_OUT_OF_RANGE;
	}

	m_opCam.uiIdxOpen = uiIdx;

	return TUCAM_Dev_Open(&m_opCam);
}

TUCAMRET TUCam::startCap()
{
	m_frame.pBuffer = NULL;
	m_frame.ucFormatGet = TUFRM_FMT_RGB888;
	m_frame.uiRsdSize = 1;

	TUCAM_Buf_Alloc(m_handle, &m_frame);

	auto ret = TUCAM_Cap_Start(m_opCam.hIdxTUCam, (UINT32)TUCCM_SEQUENCE);
	if (ret != TUCAMRET_SUCCESS) {
		return ret;
	}

	return TUCAMRET_SUCCESS;
}

void TUCam::stopCap()
{
	TUCAM_Buf_AbortWait(m_handle);
	TUCAM_Cap_Stop(m_handle);
	TUCAM_Buf_Release(m_handle);
}