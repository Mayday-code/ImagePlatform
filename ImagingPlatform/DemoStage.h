#pragma once

#include "Stage.h"
#include <iostream>

/*!
 * \class DemoStage
 * \brief Simulated stage
 *
 * \author XYH
 * \date 12 2022
 */
class DemoStage : public Stage {
public:
	DemoStage(int t_port = 0) : Stage(t_port) { }
	~DemoStage() { std::cout << "DemoStage destructed" << std::endl; }

	void init() { 
		m_state = DeviceState::REGISTER;
		std::cout << "DemoStage initialized" << std::endl; 
	}

	void moveX(int pos) override { x = pos; }
	void moveY(int pos) override { y = pos; }
	void moveZ(int pos) override { z = pos; }

	void mvrX(bool isForward) { x += XSS * (isForward ? 1 : -1); }
	void mvrY(bool isForward) { y += YSS * (isForward ? 1 : -1); }
	void mvrZ(bool isForward) { z += ZSS * 10 * (isForward ? 1 : -1); }

	std::pair<double, double> getXYPos() { return { x, y }; }
	double getZPos() { return z; }

	int getID() { return m_sessionID; }
	void setID(int t_ID) { m_sessionID = t_ID; }

	void setXSS(int t_XSS) { XSS = t_XSS; }
	void setYSS(int t_YSS) { YSS = t_YSS; }
	void setZSS(int t_ZSS) { ZSS = t_ZSS; }

private:
	int x = 0;
	int y = 0;
	int z = 0;

	int XSS;
	int YSS;
	int ZSS; // in 10nm
};