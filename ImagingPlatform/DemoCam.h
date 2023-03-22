#pragma once

#include "Camera.h"
#include <iostream>

class DemoCam : public Camera {
public:
	DemoCam();
	~DemoCam() { 
		stopSequenceAcquisition();
		std::cout << "DemoCam destructed..." << std::endl; 
	}

	virtual bool isSupportResolutionSwitching() const { return false; }

private:
	void generateSyntheticImage();

	virtual bool init() override { std::cout << "init DemoCam" << std::endl; }
	virtual bool open() override { std::cout << "Open DemoCam" << std::endl; }
	virtual bool startCapturing() override;
	virtual bool setDeviceExp(double exp_ms) override { std::cout << "ÉèÖÃÆØ¹â: " << exp_ms << "ºÁÃë" << std::endl; }
	virtual bool setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize) 
		override {
		m_hPos = hPos;
		m_vPos = vPos;
		m_width = hSize;
		m_height = vSize;
	}

	
private:
	ImgBuffer m_img;
};
