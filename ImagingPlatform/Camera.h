#pragma once

#include <mutex>
#include "CircularBuffer.h"
#include "MyDefine.h"

class Camera {
public:
	virtual ~Camera() = default;

protected:
	// members for camera
	unsigned int m_pixDepth = 0;
	//unsigned roiX_;
	//unsigned roiY_;
	double m_exp = 0;
	unsigned m_width = 0;
	unsigned m_height = 0;
	unsigned m_channel = 0;
	bool m_stop = true;

	std::mutex m_stopLock;
	CircularBuffer m_cbuf;
	DeviceState m_state = DeviceState::NOTREGISTER; // necessary member?

public:
	//Returns image buffer X - size in pixels.
	unsigned getImageWidth() const { return m_width; }
	//Returns image buffer Y - size in pixels.
	unsigned getImageHeight() const { return m_height; }

	//返回ROI的尺寸
	//virtual unsigned GetWidgetWidth() = 0;
	//Returns image buffer Y - size in pixels.
	//virtual unsigned GetWidgetHeight() = 0;

	//Returns the bit depth(dynamic range) of the pixel.
	unsigned getBitDepth() const { return m_pixDepth; }
	//Returns the channel
	unsigned getChannel() const { return m_channel; }
	//Returns the current binning factor.
	//virtual int getBinning() const = 0;
	//Sets binning factor.
	//virtual int setBinning(int binSize) = 0;

	//Sets exposure in milliseconds.
	virtual void setExposure(double exp_ms) = 0;
	//Returns the current exposure setting in milliseconds.
	double getExposure() const { return m_exp; }

	//Sets the camera Region Of Interest.
	//virtual int setROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize) = 0;
	//Returns the actual dimensions of the current ROI.
	//virtual int getROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize) = 0;
	// Resets the Region of Interest to full frame.
	//virtual int clearROI() = 0;

	//Starts Sequence Acquisition with given interval.
	virtual void startSequenceAcquisition() = 0;
	//Stops an ongoing sequence acquisition
	void stopSequenceAcquisition();
	//Flag to indicate whether Sequence Acquisition is currently running.
	//Return true when Sequence acquisition is active, false otherwise
	bool isCapturing();

	//return Buffer Top
	virtual const unsigned char* getCircularBufferTop() = 0;
	//return Buffer image count
	virtual unsigned long long getCircularBufferImageCount() const = 0;

	//Sets the camera subarray
	//virtual void SetSubArray(unsigned xSize, unsigned ySize) = 0;

	//Sets the camera flip
	//virtual void setFilp() = 0;

	//Sets the 图像ROI,img size不变，只是显示区域变小
	//virtual void setROIWidget(int x, int y, int w, int h) = 0;
	//virtual void setROIWidget_reset(int x, int y, int w, int h) = 0;

	DeviceState state() { return m_state; }
};