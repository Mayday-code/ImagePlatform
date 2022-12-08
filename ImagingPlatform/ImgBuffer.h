#pragma once

#include <memory>

class ImgBuffer
{
public:
	ImgBuffer() : m_pixels(nullptr) { }
	~ImgBuffer();

	void setPixels(const void* pixArray, unsigned, unsigned, unsigned, unsigned);
	const unsigned char* getPixels() const;

	void resize(unsigned, unsigned, unsigned, unsigned);

private:
	//ImgBuffer& operator=(const ImgBuffer&);

	unsigned char* m_pixels;
};