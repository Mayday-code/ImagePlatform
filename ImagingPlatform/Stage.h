#pragma once

#include "MyDefine.h"

class Stage {
public:
	DeviceState getState() const { return m_state; }
protected:
	DeviceState m_state = DeviceState::NOTREGISTER;
};