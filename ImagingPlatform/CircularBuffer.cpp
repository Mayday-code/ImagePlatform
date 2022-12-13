#include "CircularBuffer.h"

#include <iostream>

bool CircularBuffer::initialize(unsigned width, unsigned height, unsigned pixDepth, unsigned channels) 
{
	std::lock_guard<std::mutex> lck(m_bufferLock);

	m_width = width;
	m_height = height;
	m_pixDepth = pixDepth;
	m_channels = channels;

	// calculate the size of the entire buffer array once all images get allocated
	// the actual size at the time of the creation is going to be less, because
	// images are not allocated until pixels become available
	unsigned frameSizeKB = (m_width * m_height * m_pixDepth * m_channels) / 1024;
	unsigned cbSize = m_memorySizeMB * 1024 / frameSizeKB;

	if (cbSize == 0) {
		std::cout << "ERROR : memory footprint too small" << std::endl;
		return false;
	}

	// set a reasonable limit to circular buffer capacity 
	cbSize = cbSize > 100000 ? 100000 : cbSize;

	m_frameArray.resize(cbSize);
	std::cout << "Test resize : " << m_frameArray.size() << std::endl;
	for (unsigned i = 0; i < m_frameArray.size(); i++)
	{
		m_frameArray[i].resize(m_width, m_height, m_pixDepth, m_channels);
	}
	std::cout << "Test Resize : " << this->height() << ", " << this->width() << std::endl;
}

bool CircularBuffer::insertImage(const unsigned char* pixArray, unsigned width, unsigned height)
{
	std::unique_lock<std::mutex> lck(m_bufferLock);

	ImgBuffer* pimg;

	// check image dimensions
	if (width != m_width || height != m_height) {
		lck.unlock();
		resize(width, height);
		lck.lock();
	}

	pimg = &m_frameArray[m_insertIndex % m_frameArray.size()];

	pimg->setPixels(pixArray, m_width, m_height, m_pixDepth, m_channels);

	m_insertIndex++;
	m_insertIndex %= m_frameArray.size();

	m_imageCounter++;

	m_notEmpty.notify_all();
	return true;
}

ImgBuffer* CircularBuffer::getTopImageBuffer() 
{
	//make sure return the latest frame
	std::unique_lock<std::mutex> lck(m_bufferLock);
	m_notEmpty.wait(lck);

	//std::cout << "CircularBuffer GetNthFromTopImageBuffer : " << insertIndex_ -1 << std::endl;
	return &m_frameArray[(m_insertIndex - 1) % m_frameArray.size()];
}

bool CircularBuffer::resize(unsigned width, unsigned height) 
{
	if (width == 0 || height == 0)
		return false; // does not make sense

	if (width == m_width && height == m_height)
		if (m_frameArray.size() > 0)
			return true; // nothing to change	

	initialize(width, height, m_pixDepth, m_channels);
}