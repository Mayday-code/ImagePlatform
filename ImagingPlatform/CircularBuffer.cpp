#include "CircularBuffer.h"

#include <algorithm>

constexpr long long bytesInMB = 1 << 20;
constexpr long adjustThreshold = LONG_MAX / 2;

constexpr unsigned long maxCBSize = 10000000;

CircularBuffer::CircularBuffer(unsigned int memorySizeMB) :
	width_(0),
	height_(0),
	pixDepth_(0),
	numChannels_(0),
	imageCounter_(0),
	insertIndex_(0),
	saveIndex_(0),
	memorySizeMB_(memorySizeMB)
{
}

bool CircularBuffer::Initialize(unsigned channels, unsigned int w, unsigned int h, unsigned int pixDepth)
{
	std::lock_guard<std::mutex> lck(bufferMutex_);

	bool ret = true;
	try {
		if (w == 0 || h == 0 || pixDepth == 0 || channels == 0)
			return false; // does not make sense

		if (w == width_ && height_ == h && pixDepth_ == pixDepth && channels == numChannels_)
			if (frameArray_.size() > 0)
				return true; // nothing to change

		width_ = w;
		height_ = h;
		pixDepth_ = pixDepth;
		numChannels_ = channels;

		insertIndex_ = 0;
		saveIndex_ = 0;
		//overflow_ = false;

		// calculate the size of the entire buffer array once all images get allocated
		// the actual size at the time of the creation is going to be less, because
		// images are not allocated until pixels become available
		unsigned long frameSizeBytes = width_ * height_ * pixDepth_ * numChannels_;
		unsigned long cbSize = (unsigned long)((memorySizeMB_ * bytesInMB) / frameSizeBytes);

		if (cbSize == 0) {
			frameArray_.resize(0);
			return false; // memory footprint too small
		}

		// set a reasonable limit to circular buffer capacity 

		cbSize = std::min(cbSize, maxCBSize);

		// TODO: verify if we have enough RAM to satisfy this request
		// allocate buffers  - could conceivably throw an out-of-memory exception
		frameArray_.resize(cbSize);
		for (unsigned long i = 0; i < frameArray_.size(); i++) {
			frameArray_[i].Resize(w, h, pixDepth, channels);
		}
	}
	catch (... /* std::bad_alloc& ex */) {
		frameArray_.resize(0);
		ret = false;
	}
	return ret;
}

void CircularBuffer::Clear()
{
	std::lock_guard<std::mutex> lck(bufferMutex_);
	imageCounter_ = 0;
	insertIndex_ = 0;
	saveIndex_ = 0;
}

unsigned long CircularBuffer::GetSize() const
{
	std::lock_guard<std::mutex> lck(bufferMutex_);
	return (unsigned long)frameArray_.size();
}

unsigned long CircularBuffer::GetFreeSize() const
{
	std::lock_guard<std::mutex> lck(bufferMutex_);
	long freeSize = (long)frameArray_.size() - (insertIndex_ - saveIndex_);
	if (freeSize < 0)
		return 0;
	else
		return (unsigned long)freeSize;
}

unsigned long CircularBuffer::GetRemainingImageCount() const
{
	std::lock_guard<std::mutex> lck(bufferMutex_);
	return (unsigned long)(insertIndex_ - saveIndex_);
}


// 利用get和insert的天然互斥性，不需要条件变量，也不管队尾指向哪一帧，直接返回队头，但有可能使一帧显示多次？
ImgBuffer* CircularBuffer::GetTopImageBuffer()
{
	std::unique_lock<std::mutex> lck(bufferMutex_);
	bufferAvailable_.wait(lck, [this] {
		return insertIndex_ > 0;
		});

	long targetIndex = insertIndex_ - 1L;
	while (targetIndex < 0)
		targetIndex += (long)frameArray_.size();
	targetIndex %= frameArray_.size();

	return &frameArray_[targetIndex];
}

ImgBuffer* CircularBuffer::GetNextImageBuffer()
{
	std::unique_lock<std::mutex> lck(bufferMutex_);
	bufferAvailable_.wait(lck, [this] {
		return insertIndex_ - saveIndex_ > 0;
		});

	return &frameArray_[saveIndex_++ % frameArray_.size()];
}

bool CircularBuffer::InsertImage(const unsigned char* pixArray, unsigned int width, unsigned int height)
{
	std::unique_lock<std::mutex> lck(bufferMutex_);

	if (width != width_ || height != height_) {
		lck.unlock();
		Initialize(numChannels_, width, height, pixDepth_);
		lck.lock();
	}

	ImgBuffer* pimg = &frameArray_[insertIndex_ % frameArray_.size()];
	pimg->SetPixels(pixArray);

	imageCounter_++;
	insertIndex_++;
	if ((insertIndex_ - (long)frameArray_.size()) > adjustThreshold && (saveIndex_ - (long)frameArray_.size()) > adjustThreshold)
	{
		// adjust buffer indices to avoid overflowing integer size
		insertIndex_ -= adjustThreshold;
		saveIndex_ -= adjustThreshold;
	}

	bufferAvailable_.notify_all();
	return true;
}

