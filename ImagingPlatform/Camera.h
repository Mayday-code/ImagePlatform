#pragma once

#include <mutex>
#include <cstring>
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
public:
	virtual ~Camera() = default;

protected:
	unsigned int m_pixDepth = 0;
	//unsigned roiX_;
	//unsigned roiY_;
	double m_exp = 0;
	unsigned m_width = 0;
	unsigned m_height = 0;
	unsigned m_channel = 0;
	bool m_stop = true;

	char saveDir[256];

	std::mutex m_stopLock;
	CircularBuffer m_cbuf;
	DeviceState m_state = DeviceState::NOTREGISTER; 

public:
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
	 * \brief Set exposure in milliseconds.
	 * \param exp_ms exposure in milliseconds
	 */
	virtual void setExposure(double exp_ms) = 0;

	/*!
	 * \brief Get the current exposure setting in milliseconds.
	 */
	double getExposure() const { return m_exp; }

	//Sets the camera Region Of Interest.
	//virtual int setROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize) = 0;
	//Returns the actual dimensions of the current ROI.
	//virtual int getROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize) = 0;
	// Resets the Region of Interest to full frame.
	//virtual int clearROI() = 0;

	/*!
	 * \brief Start Sequence Acquisition.
	 */
	virtual void startSequenceAcquisition() = 0;

	/*!
	 * \brief Stop an ongoing sequence acquisition.
	 */
	void stopSequenceAcquisition();

	/*!
	 * \brief Flag to indicate whether Sequence Acquisition is currently running.
	 * \return bool : true when Sequence acquisition is active, false otherwise
	 */
	bool isCapturing();

	/*!
	 * \brief Return Buffer Top.
	 */
	virtual const unsigned char* getCircularBufferTop() = 0;

	/*!
	 * \brief Return Buffer image count.
	 */
	virtual unsigned long long getCircularBufferImageCount() const = 0;

	//Sets the camera subarray
	//virtual void SetSubArray(unsigned xSize, unsigned ySize) = 0;

	//Sets the camera flip
	//virtual void setFilp() = 0;

	//Sets the 图像ROI,img size不变，只是显示区域变小
	//virtual void setROIWidget(int x, int y, int w, int h) = 0;
	//virtual void setROIWidget_reset(int x, int y, int w, int h) = 0;

	/*!
	 * \brief Save current image.
	 * \param filename the name of this image (without extension).
	 * \return bool : True if image is saved successfully, false otherwise
	 */
	virtual bool save(const char* filename, int format) = 0;

	void setSaveDir(const char* t_dir) {
		strcpy_s(saveDir, strlen(t_dir) + 1, t_dir);
	}

	DeviceState getState() { return m_state; }
};