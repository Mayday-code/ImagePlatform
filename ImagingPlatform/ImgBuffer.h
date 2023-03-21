#pragma once

#include <cassert>

class ImgBuffer
{
public:
	ImgBuffer() : m_pixels(nullptr), m_width(0), m_height(0), m_pixDepth(0), m_channel(0) {  }
	ImgBuffer(unsigned xSize, unsigned ySize, unsigned pixDepth, unsigned channel) :
		m_pixels(0), m_width(xSize), m_height(ySize), m_pixDepth(pixDepth), m_channel(channel)
	{
		m_pixels = new unsigned char[xSize * ySize * pixDepth * channel];
		assert(m_pixels);
		memset(m_pixels, 0, xSize * ySize * pixDepth * channel);
	}
	ImgBuffer(const ImgBuffer& ib) = delete;
	ImgBuffer& operator=(const ImgBuffer& rhs) = delete;
	ImgBuffer(ImgBuffer&& rhs) noexcept :
		m_pixels(rhs.m_pixels),
		m_width(rhs.m_width),
		m_height(rhs.m_height),
		m_pixDepth(rhs.m_pixDepth),
		m_channel(rhs.m_channel)
	{
		rhs.m_pixels = nullptr;
	}

	~ImgBuffer()
	{
		if (m_pixels) {
			delete[] m_pixels;
		}
	}

	unsigned int width() const { return m_width; }
	unsigned int height() const { return m_height; }
	unsigned int depth() const { return m_pixDepth; }
	unsigned int channel() const { return m_channel; }

	// 数据大小必须是相同的维度
	void setPixels(const void* pix)
	{
		memcpy(m_pixels, pix, m_width * m_height * m_pixDepth  * m_channel);
	}

	// 设置roi的时候用？
	//void SetPixelsPadded(const void* pixArray, int paddingBytesPerLine);

	void resetPixels()
	{
		if (m_pixels)
			memset(m_pixels, 0, m_width * m_height * m_pixDepth * m_channel);
	}

	unsigned char* getPixels() { return m_pixels; }

	// 改四个维度
	void resize(unsigned hSize, unsigned vSize, unsigned pixDepth, unsigned channels)
	{
		if (m_pixels) {
			delete[] m_pixels;
		}
		m_pixels = new unsigned char[hSize * vSize * pixDepth * channels];
		m_width = hSize;
		m_height = vSize;
		m_pixDepth = pixDepth;
		m_channel = channels;
		memset(m_pixels, 0, m_width * m_height * m_pixDepth * m_channel);
	}

	// 只改两个维度
	void resize(unsigned xSize, unsigned ySize)
	{
		if (m_pixels) {
			delete[] m_pixels;
		}
		m_pixels = new unsigned char[xSize * ySize * m_pixDepth * m_channel];
		m_width = xSize;
		m_height = ySize;

		memset(m_pixels, 0, m_width * m_height * m_pixDepth * m_channel);
	}

	//bool Compatible(const ImgBuffer& img) const;

private:
	unsigned char* m_pixels;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_pixDepth;
	unsigned int m_channel;
};
