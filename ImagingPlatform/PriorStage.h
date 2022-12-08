#pragma once

#include "PriorScientificSDK.h"
#include "Stage.h"

#include <utility>

//TO do : 加入继承关系，修改接口
class PriorStage : public Stage {
public:
	PriorStage() = default;
	PriorStage(int t_port, int t_baudrate) : m_port(t_port) { }

	void init();

	//void moveX();
	//void moveY();
	//void moveZ();

	void mvrX(bool);
	void mvrY(bool);
	void mvrZ(bool);

	void close();

	std::pair<double, double> getXYPos();
	double getZPos();

	int getID() { return m_sessionID; }

	void setPort(int t_port) { m_port = t_port; }
	void setBaudrate(int t_baudrate) { }
	void setID(int t_ID) { m_sessionID = t_ID; }

	void setXSS(int t_XSS) { m_xSS = t_XSS; }
	void setYSS(int t_YSS) { m_ySS = t_YSS; }
	void setZSS(int t_ZSS) { m_zSS = t_ZSS; }
	
private:
	int cmd(const char *tx) {
		return m_ret = priorSDK.Cmd(m_sessionID, tx, m_rx);
	}

private:
	int m_sessionID;
	int m_ret;
	char m_rx[1024];
	char m_command[256];

	int m_port;

	int m_stageBusy;
	int m_zBusy;

	int m_x = 0;
	int m_y = 0;
	int m_z = 0;

	int m_xSS;
	int m_ySS;
	int m_zSS;

	//速度

	//步长单位

	PriorScientificSDK priorSDK;
};