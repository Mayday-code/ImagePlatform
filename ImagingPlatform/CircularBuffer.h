#pragma once

#include "ImgBuffer.h"
#include <vector>
#include <mutex>
#include <condition_variable>

class CircularBuffer {
public:
	CircularBuffer() = default;
	~CircularBuffer() = default;

	// each camera should only initialize() its CircularBuffer once based on its property
	// we assume that after initialize(), the pixDepth and channels of camera will not change
	bool initialize(unsigned, unsigned, unsigned, unsigned);

	// the camera is not responsible for managing the size of its CiucularBuffer
	// Instead, CiucularBuffer will change its size on its own when detecting the size of inserted image is 
	// not equal with its size
	bool insertImage(const unsigned char*, unsigned, unsigned);
	ImgBuffer getTopImageBuffer();
	bool resize(unsigned, unsigned);

	unsigned width() const { return m_width; }
	unsigned height() const { return m_height; }
	unsigned depth() const { return m_pixDepth; }
	unsigned channels() const { return m_channels; }
	unsigned long long getImageCounter() const { return m_imageCounter; }

private:
	// members
	unsigned m_memorySizeMB = 1024;
	std::vector<ImgBuffer> m_frameArray;

	// CircularBuffer is responsible for maintaining the size of its ImgBuffer 
	unsigned m_width = 0;
	unsigned m_height = 0;
	unsigned m_pixDepth = 0;// in byte
	unsigned m_channels = 0;
	unsigned m_insertIndex = 0;
	bool m_overflow = false;
	unsigned long long m_imageCounter = 0;
	std::mutex m_bufferLock;
	std::condition_variable m_notEmpty;
};