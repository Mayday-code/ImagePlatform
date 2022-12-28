#include "ImgBuffer.h"

ImgBuffer::~ImgBuffer()
{
	if (m_pixels) {
		delete[] m_pixels;
	}	
}

const unsigned char* ImgBuffer::getPixels() const
{
	return m_pixels;
}

void ImgBuffer::setPixels(const unsigned char* pix, unsigned width, unsigned height, unsigned pixDepth, unsigned channels)
{
	//must make sure pix has the same size as m_pixels
	for (size_t h = 0; h < height; h++) {
		for (size_t w = 0; w < width * channels; w += channels) {
			for (size_t c = 0; c < channels; c++) {
				m_pixels[h * width * channels + w + c] = 
					pix[(height - 1 - h) * width * channels + w + c];
			}
		}
	}
}

void ImgBuffer::resize(unsigned t_width, unsigned t_height, unsigned t_pixDepth, unsigned t_channels)
{
	// re-allocate internal buffer if it is not big enough
	if (m_pixels) delete[] m_pixels;
	m_pixels = new unsigned char[t_width * t_height * t_pixDepth * t_channels];
}