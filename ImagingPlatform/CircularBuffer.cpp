#include "CircularBuffer.h"

#include <algorithm>

constexpr long long bytesInMB = 1 << 20;
constexpr long adjustThreshold = LONG_MAX / 2;

constexpr unsigned long maxCBSize = 10000000;

CircularBuffer::CircularBuffer(unsigned int memorySizeMB) :
	m_width(0),
	m_height(0),
	m_pixDepth(0),
	m_numChannels(0),
	m_imageCounter(0),
	m_insertIndex(0),
	m_saveIndex(0),
	m_memorySizeMB(memorySizeMB)
{
}

bool CircularBuffer::initialize(unsigned channels, unsigned int w, unsigned int h, unsigned int pixDepth)
{
	std::lock_guard<std::mutex> lck(m_bufferMutex);

	bool ret = true;
	try {
		if (w == 0 || h == 0 || pixDepth == 0 || channels == 0)
			return false; // does not make sense

		if (w == m_width && m_height == h && m_pixDepth == pixDepth && channels == m_numChannels)
			if (m_frameArray.size() > 0)
				return true; // nothing to change

		m_width = w;
		m_height = h;
		m_pixDepth = pixDepth;
		m_numChannels = channels;

		m_insertIndex = 0;
		m_saveIndex = 0;
		//overflow_ = false;

		// calculate the size of the entire buffer array once all images get allocated
		// the actual size at the time of the creation is going to be less, because
		// images are not allocated until pixels become available
		unsigned long frameSizeBytes = m_width * m_height * m_pixDepth * m_numChannels;
		unsigned long cbSize = (unsigned long)((m_memorySizeMB * bytesInMB) / frameSizeBytes);

		if (cbSize == 0) {
			m_frameArray.resize(0);
			return false; // memory footprint too small
		}

		// set a reasonable limit to circular buffer capacity 

		cbSize = std::min(cbSize, maxCBSize);

		// TODO: verify if we have enough RAM to satisfy this request
		// allocate buffers  - could conceivably throw an out-of-memory exception
		m_frameArray.resize(cbSize);
		for (unsigned long i = 0; i < m_frameArray.size(); i++) {
			m_frameArray[i].resize(w, h, pixDepth, channels);
		}
	}
	catch (... /* std::bad_alloc& ex */) {
		m_frameArray.resize(0);
		ret = false;
	}
	return ret;
}

void CircularBuffer::clear()
{
	std::lock_guard<std::mutex> lck(m_bufferMutex);
	m_imageCounter = 0;
	m_insertIndex = 0;
	m_saveIndex = 0;
}

unsigned long CircularBuffer::getSize() const
{
	std::lock_guard<std::mutex> lck(m_bufferMutex);
	return (unsigned long)m_frameArray.size();
}

unsigned long CircularBuffer::getFreeSize() const
{
	std::lock_guard<std::mutex> lck(m_bufferMutex);
	long freeSize = (long)m_frameArray.size() - (m_insertIndex - m_saveIndex);
	if (freeSize < 0)
		return 0;
	else
		return (unsigned long)freeSize;
}

unsigned long CircularBuffer::getRemainingImageCount() const
{
	std::lock_guard<std::mutex> lck(m_bufferMutex);
	return (unsigned long)(m_insertIndex - m_saveIndex);
}


// 利用get和insert的天然互斥性，不需要条件变量，也不管队尾指向哪一帧，直接返回队头，但有可能使一帧显示多次？
ImgBuffer* CircularBuffer::getTopImageBuffer()
{
	std::unique_lock<std::mutex> lck(m_bufferMutex);
	m_bufferAvailable.wait(lck, [this] {
		return m_insertIndex > 0;
		});

	long targetIndex = m_insertIndex - 1L;
	while (targetIndex < 0)
		targetIndex += (long)m_frameArray.size();
	targetIndex %= m_frameArray.size();

	return &m_frameArray[targetIndex];
}

ImgBuffer* CircularBuffer::getNextImageBuffer()
{
	std::unique_lock<std::mutex> lck(m_bufferMutex);
	m_bufferAvailable.wait(lck, [this] {
		return m_insertIndex - m_saveIndex > 0;
		});

	return &m_frameArray[m_saveIndex++ % m_frameArray.size()];
}

bool CircularBuffer::insertImage(const unsigned char* pixArray, unsigned int width, unsigned int height)
{
	std::unique_lock<std::mutex> lck(m_bufferMutex);

	if (width != m_width || height != m_height) {
		lck.unlock();
		initialize(m_numChannels, width, height, m_pixDepth);
		lck.lock();
	}

	ImgBuffer* pimg = &m_frameArray[m_insertIndex % m_frameArray.size()];
	pimg->setPixels(pixArray);

	m_imageCounter++;
	m_insertIndex++;
	if ((m_insertIndex - (long)m_frameArray.size()) > adjustThreshold && (m_saveIndex - (long)m_frameArray.size()) > adjustThreshold)
	{
		// adjust buffer indices to avoid overflowing integer size
		m_insertIndex -= adjustThreshold;
		m_saveIndex -= adjustThreshold;
	}

	m_bufferAvailable.notify_all();
	return true;
}

