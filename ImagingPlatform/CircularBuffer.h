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

	unsigned GetMemorySizeMB() const { return memorySizeMB_; }

	bool Initialize(unsigned channels, unsigned int w, unsigned int h, unsigned int pixDepth);

	// ��ջ�������ֻ�轫�������ã���������޸�frameArray_������ݣ�
	void Clear();
	unsigned long GetSize() const;
	unsigned long GetFreeSize() const;
	unsigned long GetRemainingImageCount() const;
	unsigned long long GetImageCounter() const { std::lock_guard<std::mutex> lck(bufferMutex_); return imageCounter_; }

	unsigned int Width() const { std::lock_guard<std::mutex> lck(bufferMutex_); return width_; }
	unsigned int Height() const { std::lock_guard<std::mutex> lck(bufferMutex_); return height_; }
	unsigned int Depth() const { std::lock_guard<std::mutex> lck(bufferMutex_); return pixDepth_; }

	ImgBuffer* GetTopImageBuffer();
	ImgBuffer* GetNextImageBuffer();
	bool InsertImage(const unsigned char* pixArray, unsigned int width, unsigned int height);

	long getInsertIndex() const { std::lock_guard<std::mutex> lck(bufferMutex_); return insertIndex_; }
	long getSaveIndex() const { std::lock_guard<std::mutex> lck(bufferMutex_); return saveIndex_; }

private:
	unsigned int width_;
	unsigned int height_;
	unsigned int pixDepth_;
	unsigned int numChannels_;
	unsigned long long imageCounter_;

	// Invariants:
	// 0 <= saveIndex_ <= insertIndex_
	// insertIndex_ - saveIndex_ <= frameArray_.size()
	// ���֮��ǰһ�ֵ����ݶ��ᱻ�ƻ�
	long insertIndex_;
	long saveIndex_;

	unsigned long memorySizeMB_;

	//bool overflow_;
	std::vector<ImgBuffer> frameArray_;
	mutable std::mutex bufferMutex_;
	mutable std::condition_variable bufferAvailable_;
};
