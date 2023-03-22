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
	 * \brief 初始化api接口
	 */
	virtual bool init() = 0;


	/*!
	 * \brief 打开相机
	 */
	virtual bool open() = 0;

	/*!
	 * \brief 开始采集
	 */
	virtual bool startCapturing() = 0;

	/*!
	 * \brief 设置曝光，单位为毫秒。需要先设置曝光值，再查询真实值
	 */
	virtual bool setDeviceExp(double exp_ms) = 0;

	/*!
	 * \brief 设置设备的ROI，这个函数还需要负责同步相机真实宽高、hPos、vPos和
	 * 相机类的m_width、m_height、m_hPos、m_vPos属性成员
	 */
	virtual bool setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) = 0;

public:
	virtual ~Camera() = default;

	/*!
	 * \brief 相机是否支持分辨率切换
	 */
	virtual bool isSupportResolutionSwitching() const = 0;

	/*!
	 * \brief Set exposure in milliseconds.
	 * \param exp_ms exposure in milliseconds
	 */
	void setExposure(double exp_ms) {
		if (getState() == CameraState::OFFLINE) {
			std::cout << "相机未连接" << std::endl;
			return;
		}

		bool isContinue = false;
		if (getState() == CameraState::LIVING) {
			isContinue = true;
			stopSequenceAcquisition();
		}

		std::cout << "设置曝光时间：" << exp_ms << "ms" << std::endl;

		if (!setDeviceExp(exp_ms)) {
			std::cout << "设置曝光失败" << std::endl;
			//设置失败的话要再查询当前的属性值，然后同步到UI
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
			std::cout << "相机未连接" << std::endl;
			return;
		}

		bool isContinue = false;
		if (getState() == CameraState::LIVING) {
			isContinue = true;
			stopSequenceAcquisition();
		}

		std::cout << "设置ROI:\n"
			"	hPOs: " << hPos << "\n"
			"	vPos: " << vPos << "\n"
			"	hSize: " << hSize << "\n"
			"	vSize: " << vSize << std::endl;

		if (!setDeviceROI(hPos, vPos, hSize, vSize)) {
			std::cout << "设置ROI失败" << std::endl;
			//设置失败的话要再查询当前的属性值，然后同步到UI
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
			std::cout << "相机未连接" << std::endl;
			return;
		}

		if (getState() == CameraState::LIVING) {
			return;
		}

		std::cout << "开始采图" << std::endl;

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
			std::cout << "相机未连接" << std::endl;
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
	 * \brief 返回缓冲队列中的最后一帧
	 */
	ImgBuffer* getNextBuffer() { return m_cbuf.getNextImageBuffer(); }

	/*!
	 * \brief 可在测试时使用，用于分析缓冲队列队头队尾的情况
	 */
	long getInsertIndex() const { return m_cbuf.getInsertIndex(); }

	/*!
	 * \brief 可在测试时使用，用于分析缓冲队列队头队尾的情况
	 */
	long getSaveIndex() const { return m_cbuf.getSaveIndex(); }

	/*!
	 * \brief 清空缓冲区
	 */
	void clearCircularBuffer() { m_cbuf.clear(); }

	/*!
	 * \brief 返回缓冲区生命周期内累计插入图像数量
	 */
	unsigned long long getCircularBufferImageCount() const { return m_cbuf.getImageCounter(); }

	//Sets the camera subarray
	//virtual void SetSubArray(unsigned xSize, unsigned ySize) = 0;

	//Sets the camera flip
	//virtual void setFilp() = 0;

	//Sets the 图像ROI,img size不变，只是显示区域变小
	//virtual void setROIWidget(int x, int y, int w, int h) = 0;
	//virtual void setROIWidget_reset(int x, int y, int w, int h) = 0;

	CameraState getState() { std::lock_guard<std::mutex> lck(m_stateMutex); return m_state; }
};