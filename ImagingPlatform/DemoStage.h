#pragma once

#include "Stage.h"
#include <iostream>

class DemoStage : public Stage {
public:
	DemoStage(int t_port = 0, int t_baudrate = 0) : Stage(t_port, t_baudrate) { }
	~DemoStage() { std::cout << "DemoStage destructed" << std::endl; }

	void init() { 
		m_state = DeviceState::REGISTER;
		std::cout << "DemoStage initialized" << std::endl; 
	}

	//void moveX();
	//void moveY();
	//void moveZ();

	void mvrX(bool) { std::cout << "DemoStage mvrX" << std::endl; }
	void mvrY(bool) { std::cout << "DemoStage mvrY" << std::endl; }
	void mvrZ(bool) { std::cout << "DemoStage mvrZ" << std::endl; }

	std::pair<double, double> getXYPos() { 
		return { 3.14, 3.14 };
	}

	double getZPos() { 
		return 3.14;
	}

	int getID() { return m_sessionID; }
	void setID(int t_ID) { m_sessionID = t_ID; }

	void setXSS(int t_XSS) { std::cout << "DemoStage setXSS" << std::endl; }
	void setYSS(int t_YSS) { std::cout << "DemoStage setYSS" << std::endl; }
	void setZSS(int t_ZSS) { std::cout << "DemoStage setZSS" << std::endl; }
};