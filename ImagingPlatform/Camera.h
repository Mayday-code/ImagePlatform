#pragma once

#include <mutex>
#include <cstring>
#include <atomic>
#include "CircularBuffer.h"
#include "MyDefine.h"

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
	 * \brief �����
	 */
	virtual bool open() = 0;

public:
	virtual ~Camera() = default;

	/*!
	 * \brief Set exposure in milliseconds.
	 * \param exp_ms exposure in milliseconds
	 */
	virtual void setExposure(double exp_ms) = 0;

	/*!
	 * \brief Sets the camera Region Of Interest.
	 */
	virtual void setROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) = 0;

	/*!
	 * \brief Start Sequence Acquisition.
	 */
	virtual void startSequenceAcquisition() = 0;

	/*!
	 * \brief Get image width - size in pixels.
	 */
	unsigned getImageWidth() const { return m_width; }

	/*!
	 * \brief Get image height - size in pixels.
	 */
	unsigned getImageHeight() const { return m_height; }

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
		std::unique_lock<std::mutex> lck(m_stateMutex);
		m_state = CameraState::ONLINE;
		lck.unlock();
		std::this_thread::sleep_for(200ms);
	}

	/*!
	 * \brief Flag to indicate whether Sequence Acquisition is currently running.
	 * \return bool : true when Sequence acquisition is active, false otherwise
	 */
	bool isCapturing() const { std::lock_guard<std::mutex> lck(m_stateMutex); return m_state == CameraState::LIVING; }

	/*!
	 * \brief Return Buffer Top.
	 */
	ImgBuffer* getTopBuffer() { return m_cbuf.GetTopImageBuffer(); }

	/*!
	 * \brief ���ػ�������е����һ֡
	 */
	ImgBuffer* getNextBuffer() { return m_cbuf.GetNextImageBuffer(); }

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
	void clearCircularBuffer() { m_cbuf.Clear(); }

	/*!
	 * \brief ���ػ����������������ۼƲ���ͼ������
	 */
	unsigned long long getCircularBufferImageCount() const { return m_cbuf.GetImageCounter(); }

	//Sets the camera subarray
	//virtual void SetSubArray(unsigned xSize, unsigned ySize) = 0;

	//Sets the camera flip
	//virtual void setFilp() = 0;

	//Sets the ͼ��ROI,img size���䣬ֻ����ʾ�����С
	//virtual void setROIWidget(int x, int y, int w, int h) = 0;
	//virtual void setROIWidget_reset(int x, int y, int w, int h) = 0;

	CameraState getState() { std::lock_guard<std::mutex> lck(m_stateMutex); return m_state; }
};