#pragma once

#include "ImgBuffer.h"
#include <vector>
#include <mutex>
#include <condition_variable>

class CircularBuffer
{
public:
	CircularBuffer(unsigned int memorySizeMB = 2048);
	~CircularBuffer() = default;

	unsigned getMemorySizeMB() const { return m_memorySizeMB; }

	bool initialize(unsigned channels, unsigned int w, unsigned int h, unsigned int pixDepth);

	// ��ջ�������ֻ�轫�������ã���������޸�m_frameArray������ݣ�
	void clear();
	unsigned long getSize() const;
	unsigned long getFreeSize() const;
	unsigned long getRemainingImageCount() const;
	unsigned long long getImageCounter() const { std::lock_guard<std::mutex> lck(m_bufferMutex); return m_imageCounter; }

	unsigned int width() const { std::lock_guard<std::mutex> lck(m_bufferMutex); return m_width; }
	unsigned int height() const { std::lock_guard<std::mutex> lck(m_bufferMutex); return m_height; }
	unsigned int depth() const { std::lock_guard<std::mutex> lck(m_bufferMutex); return m_pixDepth; }

	ImgBuffer* getTopImageBuffer();
	ImgBuffer* getNextImageBuffer();
	bool insertImage(const unsigned char* pixArray, unsigned int width, unsigned int height);

	long getInsertIndex() const { std::lock_guard<std::mutex> lck(m_bufferMutex); return m_insertIndex; }
	long getSaveIndex() const { std::lock_guard<std::mutex> lck(m_bufferMutex); return m_saveIndex; }

private:
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_pixDepth;
	unsigned int m_numChannels;
	unsigned long long m_imageCounter;

	// Invariants:
	// 0 <= m_saveIndex <= m_insertIndex
	// m_insertIndex - m_saveIndex <= m_frameArray.size()
	// ���֮��ǰһ�ֵ����ݶ��ᱻ�ƻ�
	long m_insertIndex;
	long m_saveIndex;

	unsigned long m_memorySizeMB;

	//bool overflow_;
	std::vector<ImgBuffer> m_frameArray;
	mutable std::mutex m_bufferMutex;
	mutable std::condition_variable m_bufferAvailable;
};
