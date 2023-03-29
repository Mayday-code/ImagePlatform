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

	//��ʼ����Ҫ��������ʾͬ��
	setROI(0, 0, 5472, 3648);
	setExposure(200);

	m_pixDepth = 1;
	m_channel = 3;

	m_cbuf.initialize(m_channel, m_width, m_height, m_pixDepth);

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

	TUCAM_Capa_SetValue(m_handle, TUIDC_RESOLUTION, 0);
	TUCAM_Capa_SetValue(m_handle, TUIDC_VERTICAL, 1);

	std::cout << "������ӳɹ�" << std::endl;
}

TUCam::~TUCam()
{
	stopSequenceAcquisition();
	TUCAM_Dev_Close(m_handle);
	printf("������Ͽ�����\n");
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
		std::cout << "��ʼ������!!!�������: " << std::hex << ret << std::endl;
		return false;
	}

	// һ��ʼprintf()�Ὣ���ݴ���IO���������������л��壬
	// �л�����ص�������\nʱ������C������������ˢ�µ��ں˻�����
	printf("��⵽ %d ̨���\n", m_itApi.uiCamCount);
	return true;
}

bool TUCam::open()
{
	m_opCam.uiIdxOpen = 0;
	auto ret = TUCAM_Dev_Open(&m_opCam);
	if (TUCAMRET_SUCCESS != ret) {
		printf("���������!!!�������: %x\n", ret);
		return false;
	}
	m_handle = m_opCam.hIdxTUCam;
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
		printf("�޷������ɼ�!!!�������: %x\n", ret);
		return false;
	}

	std::lock_guard<std::mutex> lck(m_stateMutex);
	m_state = CameraState::LIVING;

	std::thread thread_capture([this] {
		while (isCapturing()) {
			auto ret = TUCAM_Buf_WaitForFrame(m_opCam.hIdxTUCam, &m_frame);
			if (TUCAMRET_SUCCESS == ret) {
				m_cbuf.insertImage(m_frame.pBuffer + m_frame.usOffset, m_frame.usWidth, m_frame.usHeight);
			} else {
				printf("ץȡ֡����!!!�������: %x\n", ret);
			}
		}
		printf("�˳���ͼ\n");

		// ������ɼ�����ֹͣ
		stopCap();
	});
	thread_capture.detach();
}

bool TUCam::setDeviceExp(double exp_ms)
{
	TUCAMRET ret = TUCAM_Prop_SetValue(m_handle, TUIDP_EXPOSURETM, exp_ms);
	TUCAM_Prop_GetValue(m_handle, TUIDP_EXPOSURETM, &m_exp);
	if (ret != TUCAMRET_SUCCESS) {
		printf("�����ع����!!!�������: %x\n", ret);
		return false;
	}
	return true;
}

bool TUCam::setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize)
{
	TUCAM_ROI_ATTR roiAttr;
	roiAttr.bEnable = TRUE;
	roiAttr.nVOffset = vPos;
	roiAttr.nHOffset = hPos;
	roiAttr.nWidth = hSize;
	roiAttr.nHeight = vSize;
	
	auto ret = TUCAM_Cap_SetROI(m_opCam.hIdxTUCam, roiAttr);

	TUCAM_ROI_ATTR getROI;
	getROI.bEnable = TRUE;
	TUCAM_Cap_GetROI(m_handle, &getROI);

	m_vPos = getROI.nVOffset;
	m_hPos = getROI.nHOffset;
	m_width = getROI.nWidth;
	m_height = getROI.nHeight;

	if (TUCAMRET_SUCCESS != ret) {
		printf("����ROI����!!!�������: %x\n", ret);
		return false;
	}

	return true;
}

bool TUCam::setDeviceRes(int index)
{
	if (index == 0) {
		TUCAM_Capa_SetValue(m_handle, TUIDC_RESOLUTION, 0);
		setROI(0, 0, 5472, 3648);
	}
	else if (index == 1) {
		TUCAM_Capa_SetValue(m_handle, TUIDC_RESOLUTION, 1);
		setROI(0, 0, 2736, 1824);
	}
	else if (index == 2) {
		TUCAM_Capa_SetValue(m_handle, TUIDC_RESOLUTION, 2);
		setROI(0, 0, 1824, 1216);
	}

	return true;
}

void TUCam::stopCap()
{
	TUCAM_Buf_AbortWait(m_handle);
	TUCAM_Cap_Stop(m_handle);
	TUCAM_Buf_Release(m_handle);
}