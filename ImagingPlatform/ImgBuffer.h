#pragma once

#include <cassert>

class ImgBuffer
{
public:
	ImgBuffer() : pixels_(nullptr), width_(0), height_(0), pixDepth_(0), channel_(0) {  }
	ImgBuffer(unsigned xSize, unsigned ySize, unsigned pixDepth, unsigned channel) :
		pixels_(0), width_(xSize), height_(ySize), pixDepth_(pixDepth), channel_(channel)
	{
		pixels_ = new unsigned char[xSize * ySize * pixDepth * channel];
		assert(pixels_);
		memset(pixels_, 0, xSize * ySize * pixDepth * channel);
	}
	ImgBuffer(const ImgBuffer& ib) = delete;
	ImgBuffer& operator=(const ImgBuffer& rhs) = delete;
	ImgBuffer(ImgBuffer&& rhs) noexcept :
		pixels_(rhs.pixels_),
		width_(rhs.width_),
		height_(rhs.height_),
		pixDepth_(rhs.pixDepth_),
		channel_(rhs.channel_)
	{
		rhs.pixels_ = nullptr;
	}

	~ImgBuffer()
	{
		if (pixels_) {
			delete[] pixels_;
		}
	}

	unsigned int Width() const { return width_; }
	unsigned int Height() const { return height_; }
	unsigned int Depth() const { return pixDepth_; }
	unsigned int Channel() const { return channel_; }

	// 数据大小必须是相同的维度
	void SetPixels(const void* pix)
	{
		memcpy(pixels_, pix, width_ * height_ * pixDepth_  * channel_);
	}

	// 设置roi的时候用？
	//void SetPixelsPadded(const void* pixArray, int paddingBytesPerLine);

	void ResetPixels()
	{
		if (pixels_)
			memset(pixels_, 0, width_ * height_ * pixDepth_ * channel_);
	}

	unsigned char* GetPixels() { return pixels_; }

	// 改四个维度
	void Resize(unsigned xSize, unsigned ySize, unsigned pixDepth, unsigned channels)
	{
		if (pixels_) {
			delete[] pixels_;
		}
		pixels_ = new unsigned char[xSize * ySize * pixDepth * channels];
		width_ = xSize;
		height_ = ySize;
		pixDepth_ = pixDepth;
		channel_ = channels;
		memset(pixels_, 0, width_ * height_ * pixDepth_ * channel_);
	}

	// 只改两个维度
	void Resize(unsigned xSize, unsigned ySize)
	{
		if (pixels_) {
			delete[] pixels_;
		}
		pixels_ = new unsigned char[xSize * ySize * pixDepth_ * channel_];
		width_ = xSize;
		height_ = ySize;

		memset(pixels_, 0, width_ * height_ * pixDepth_ * channel_);
	}

	//bool Compatible(const ImgBuffer& img) const;

private:
	unsigned char* pixels_;
	unsigned int width_;
	unsigned int height_;
	unsigned int pixDepth_;
	unsigned int channel_;
};
