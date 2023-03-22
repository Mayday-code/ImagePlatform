#pragma once

#include <mutex>
#include <iostream>
#include <atomic>
#include "CircularBuffer.h"
#include "MyDefine.h"

using namespace std::literals;

/*!
 * \class Camera
 * \brief Base class for all cameras.
 *
 * \author XYH
 * \date 12 2022
 */
class Camera {
protected:
	unsigned int m_pixDepth = 0;
	//unsigned roiX_;
	//unsigned roiY_;
	double m_exp = 0;
	unsigned m_hPos = 0;
	unsigned m_vPos = 0;
	unsigned m_width = 0;
	unsigned m_height = 0;
	unsigned m_channel = 0;
	mutable std::mutex m_stateMutex;

	CircularBuffer m_cbuf;
	CameraState m_state = CameraState::OFFLINE;

	/*!
	 * \brief ��ʼ��api�ӿ�
	 */
	virtual bool init() = 0;


	/*!
	 * \brief �����
	 */
	virtual bool open() = 0;

	/*!
	 * \brief ��ʼ�ɼ�
	 */
	virtual bool startCapturing() = 0;

	/*!
	 * \brief �����ع⣬��λΪ���롣��Ҫ�������ع�ֵ���ٲ�ѯ��ʵֵ
	 */
	virtual bool setDeviceExp(double exp_ms) = 0;

	/*!
	 * \brief �����豸��ROI�������������Ҫ����ͬ�������ʵ��ߡ�hPos��vPos��
	 * ������m_width��m_height��m_hPos��m_vPos���Գ�Ա
	 */
	virtual bool setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) = 0;

public:
	virtual ~Camera() = default;

	/*!
	 * \brief ����Ƿ�֧�ֱַ����л�
	 */
	virtual bool isSupportResolutionSwitching() const = 0;

	/*!
	 * \brief Set exposure in milliseconds.
	 * \param exp_ms exposure in milliseconds
	 */
	void setExposure(double exp_ms) {
		if (getState() == CameraState::OFFLINE) {
			std::cout << "���δ����" << std::endl;
			return;
		}

		bool isContinue = false;
		if (getState() == CameraState::LIVING) {
			isContinue = true;
			stopSequenceAcquisition();
		}

		std::cout << "�����ع�ʱ�䣺" << exp_ms << "ms" << std::endl;

		if (!setDeviceExp(exp_ms)) {
			std::cout << "�����ع�ʧ��" << std::endl;
			//����ʧ�ܵĻ�Ҫ�ٲ�ѯ��ǰ������ֵ��Ȼ��ͬ����UI
		}

		if (isContinue) {
			startSequenceAcquisition();
		}
	}

	/*!
	 * \brief Sets the camera Region Of Interest.
	 */
	void setROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) {
		if (getState() == CameraState::OFFLINE) {
			std::cout << "���δ����" << std::endl;
			return;
		}

		bool isContinue = false;
		if (getState() == CameraState::LIVING) {
			isContinue = true;
			stopSequenceAcquisition();
		}

		std::cout << "����ROI:\n"
			"	hPOs: " << hPos << "\n"
			"	vPos: " << vPos << "\n"
			"	hSize: " << hSize << "\n"
			"	vSize: " << vSize << std::endl;

		if (!setDeviceROI(hPos, vPos, hSize, vSize)) {
			std::cout << "����ROIʧ��" << std::endl;
			//����ʧ�ܵĻ�Ҫ�ٲ�ѯ��ǰ������ֵ��Ȼ��ͬ����UI
		}
		else {
			m_width = hSize;
			m_height = vSize;
		}

		if (isContinue) {
			startSequenceAcquisition();
		}
	}

	/*!
	 * \brief Start Sequence Acquisition.
	 */
	void startSequenceAcquisition() {
		if (getState() == CameraState::OFFLINE) {
			std::cout << "���δ����" << std::endl;
			return;
		}

		if (getState() == CameraState::LIVING) {
			return;
		}

		std::cout << "��ʼ��ͼ" << std::endl;

		startCapturing();
	}

	/*!
	 * \brief Get image width - size in pixels.
	 */
	unsigned getImageWidth() const { return m_width; }

	/*!
	 * \brief Get image height - size in pixels.
	 */
	unsigned getImageHeight() const { return m_height; }

	unsigned getHPos() const { return m_hPos; }

	unsigned getVPos() const { return m_vPos; }

	//Gets the size of ROI
	//virtual unsigned GetWidgetWidth() = 0;
	//Returns image buffer Y - size in pixels.
	//virtual unsigned GetWidgetHeight() = 0;

	/*!
	 * \brief Get the bit depth(dynamic range) of the pixel.
	 */
	unsigned getBitDepth() const { return m_pixDepth; }

	/*!
	 * \brief Get the size of channels.
	 */
	unsigned getChannel() const { return m_channel; }

	//Returns the current binning factor.
	//virtual int getBinning() const = 0;
	//Sets binning factor.
	//virtual int setBinning(int binSize) = 0;

	/*!
	 * \brief Get the current exposure setting in milliseconds.
	 */
	double getExposure() const { return m_exp; }

	//Returns the actual dimensions of the current ROI.
	//virtual int getROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize) = 0;
	// Resets the Region of Interest to full frame.
	//virtual int clearROI() = 0;

	/*!
	 * \brief Stop an ongoing sequence acquisition.
	 */
	void stopSequenceAcquisition() {
		if (getState() == CameraState::OFFLINE) {
			std::cout << "���δ����" << std::endl;
			return;
		}
		std::unique_lock<std::mutex> lck(m_stateMutex);
		m_state = CameraState::ONLINE;
		lck.unlock();
		std::this_thread::sleep_for(50ms);
	}

	/*!
	 * \brief Flag to indicate whether Sequence Acquisition is currently running.
	 * \return bool : true when Sequence acquisition is active, false otherwise
	 */
	bool isCapturing() const { std::lock_guard<std::mutex> lck(m_stateMutex); return m_state == CameraState::LIVING; }

	/*!
	 * \brief Return Buffer Top.
	 */
	ImgBuffer* getTopBuffer() { return m_cbuf.getTopImageBuffer(); }

	/*!
	 * \brief ���ػ�������е����һ֡
	 */
	ImgBuffer* getNextBuffer() { return m_cbuf.getNextImageBuffer(); }

	/*!
	 * \brief ���ڲ���ʱʹ�ã����ڷ���������ж�ͷ��β�����
	 */
	long getInsertIndex() const { return m_cbuf.getInsertIndex(); }

	/*!
	 * \brief ���ڲ���ʱʹ�ã����ڷ���������ж�ͷ��β�����
	 */
	long getSaveIndex() const { return m_cbuf.getSaveIndex(); }

	/*!
	 * \brief ��ջ�����
	 */
	void clearCircularBuffer() { m_cbuf.clear(); }

	/*!
	 * \brief ���ػ����������������ۼƲ���ͼ������
	 */
	unsigned long long getCircularBufferImageCount() const { return m_cbuf.getImageCounter(); }

	//Sets the camera subarray
	//virtual void SetSubArray(unsigned xSize, unsigned ySize) = 0;

	//Sets the camera flip
	//virtual void setFilp() = 0;

	//Sets the ͼ��ROI,img size���䣬ֻ����ʾ�����С
	//virtual void setROIWidget(int x, int y, int w, int h) = 0;
	//virtual void setROIWidget_reset(int x, int y, int w, int h) = 0;

	CameraState getState() { std::lock_guard<std::mutex> lck(m_stateMutex); return m_state; }
};