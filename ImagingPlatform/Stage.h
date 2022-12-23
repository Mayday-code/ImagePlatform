#pragma once

#include "MyDefine.h"
#include <utility>

// To use Stage, first construct it, and then init it
class Stage {
public:
	Stage(int t_port, int t_baudrate) : m_port(t_port), m_baudrate(t_baudrate) { }
	virtual ~Stage() = default;

	virtual void init() = 0;

	//void moveX();
	//void moveY();
	//void moveZ();

	virtual void mvrX(bool) = 0;
	virtual void mvrY(bool) = 0;
	virtual void mvrZ(bool) = 0;

	virtual std::pair<double, double> getXYPos() = 0;
	virtual double getZPos() = 0;

	int getID() { return m_sessionID; }
	void setID(int t_ID) { m_sessionID = t_ID; }

	virtual void setXSS(int t_XSS) = 0;
	virtual void setYSS(int t_YSS) = 0;
	virtual void setZSS(int t_ZSS) = 0;

	DeviceState getState() const { return m_state; }
protected:
	DeviceState m_state = DeviceState::NOTREGISTER;

	int m_sessionID;

	int m_port;
	int m_baudrate;
};