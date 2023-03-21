#include "TUCam.h"

#include <QMessageBox>
#include <iostream>
#include <chrono>
#include <thread>
#include <QApplication>
#include <stdio.h>

TUCam::TUCam()
{
	if (!init()) return;
	if (!open()) return;

	m_state = CameraState::ONLINE;

	//初始设置要与界面的显示同步
	//测试鑫图相机的各个ROI选项
	//setROI(0, 0, 900, 600);
	setExposure(200);

	//鑫图相机目前没有实现setDeviceROI()函数，这里暂时需要手动设置
	m_height = 1216;
	m_width = 1824;
	m_pixDepth = 1;
	m_channel = 3;

	m_cbuf.initialize(m_channel, m_width, m_height, m_pixDepth);

	m_handle = m_opCam.hIdxTUCam;

	TUCAM_PROP_ATTR attrProp;
	attrProp.nIdxChn = 0;    // Current channel 
	attrProp.idProp = TUIDP_EXPOSURETM;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_handle, &attrProp)) {
		// Exposure time range 
		std::cout << "Minimum exposure time : " << attrProp.dbValMin << std::endl;
		std::cout << "Maximum exposure time : " << attrProp.dbValMax << std::endl;
		std::cout << "Default exposure time : " << attrProp.dbValDft << std::endl;
		std::cout << "Exposure time step : " << attrProp.dbValStep << std::endl;
	}

	// get exposure for testing
	double dbVal = 1.0f;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetValue(m_handle, TUIDP_EXPOSURETM, &dbVal)) {
		std::cout << "Current exposure : " << dbVal << std::endl;
	}

	TUCAM_Capa_SetValue(m_handle, TUIDC_RESOLUTION, 2);

	std::cout << "相机连接成功" << std::endl;
}

TUCam::~TUCam()
{
	if (nullptr != m_handle) {
		stopCap();
		TUCAM_Dev_Close(m_handle);
	}
	printf("与相机断开连接\n");

	TUCAM_Api_Uninit();
}

bool TUCam::init()
{
	/* Get the current directory */
	m_itApi.uiCamCount = 0;
	char configPath[256];
	strcpy_s(configPath, sizeof configPath, QApplication::applicationDirPath().toStdString().c_str());
	m_itApi.pstrConfigPath = configPath;

	auto ret = TUCAM_Api_Init(&m_itApi);
	if (TUCAMRET_SUCCESS != ret) {
		printf("初始化出错!!!错误代码: %x\n", ret);
		return false;
	}

	printf("检测到 %d 台相机\n", m_itApi.uiCamCount);
	return true;
}

bool TUCam::open()
{
	m_opCam.uiIdxOpen = 0;
	auto ret = TUCAM_Dev_Open(&m_opCam);
	if (TUCAMRET_SUCCESS != ret) {
		printf("打开相机出错!!!错误代码: %x\n", ret);
		return false;
	}
	return true;
}

bool TUCam::startCapturing()
{
	m_frame.pBuffer = NULL;
	m_frame.ucFormatGet = TUFRM_FMT_RGB888;
	m_frame.uiRsdSize = 1;

	TUCAM_Buf_Alloc(m_handle, &m_frame);

	auto ret = TUCAM_Cap_Start(m_opCam.hIdxTUCam, (UINT32)TUCCM_SEQUENCE);
	if (TUCAMRET_SUCCESS != ret) {
		printf("无法启动采集!!!错误代码: %x\n", ret);
		return false;
	}

	std::lock_guard<std::mutex> lck(m_stateMutex);
	m_state = CameraState::LIVING;

	std::thread thread_capture([this] {
		while (isCapturing()) {
			auto ret = TUCAM_Buf_WaitForFrame(m_opCam.hIdxTUCam, &m_frame);
			if (TUCAMRET_SUCCESS == ret) {
				m_cbuf.insertImage(m_frame.pBuffer + m_frame.usOffset, m_frame.usWidth, m_frame.usHeight);
#ifndef _DEBUG
				static unsigned index = 0;
				std::cout << "inserted " << index++ << std::endl;
#endif // !_DEBUG
			} else {
				printf("抓取帧出错!!!错误代码: %x\n", ret);
			}
		}
		std::cout << "退出采图" << std::endl;

		// 将相机采集真正停止
		stopCap();
	});
	thread_capture.detach();
}

bool TUCam::setDeviceExp(double exp_ms)
{
	TUCAMRET ret = TUCAM_Prop_SetValue(m_handle, TUIDP_EXPOSURETM, exp_ms);
	if (ret != TUCAMRET_SUCCESS) {
		printf("设置曝光出错!!!错误代码: %x\n", ret);
		return false;
	}
	return true;
}

bool TUCam::setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize)
{
	printf("未实现此功能\n");
	return false;
}

void TUCam::stopCap()
{
	TUCAM_Buf_AbortWait(m_handle);
	TUCAM_Cap_Stop(m_handle);
	TUCAM_Buf_Release(m_handle);
}