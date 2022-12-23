#pragma once

#include "PriorScientificSDK.h"
#include "Stage.h"

#include <utility>

class PriorStage : public Stage {
public:
	PriorStage(int t_port, int t_baudrate) : Stage(t_port, t_baudrate) { }
	~PriorStage() { close(); }

	void init() override;

	//void moveX();
	//void moveY();
	//void moveZ();

	void mvrX(bool) override;
	void mvrY(bool) override;
	void mvrZ(bool) override;

	void close();

	std::pair<double, double> getXYPos() override;
	double getZPos() override;

	void setXSS(int t_XSS) override { m_xSS = t_XSS; }
	void setYSS(int t_YSS) override { m_ySS = t_YSS; }
	void setZSS(int t_ZSS) override { m_zSS = t_ZSS; }
	
private:
	int cmd(const char *tx) {
		return m_ret = priorSDK.Cmd(m_sessionID, tx, m_rx);
	}

private:
	int m_ret;
	char m_rx[1024];
	char m_command[256];

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