#pragma once

#include "PriorScientificSDK.h"
#include "Stage.h"

#include <utility>

/*!
 * \class PriorStage
 * \brief For Prior stage
 *
 * \author XYH
 * \date 12 2022
 */
class PriorStage : public Stage {
public:
	PriorStage(int t_port) : Stage(t_port) { }
	~PriorStage() { close(); }

	void init() override;

	void moveXY(int xpos, int ypos) override;
	void moveZ(int pos) override;

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
	/*!
	 * \brief Send command to Prior stage.
	 * \param tx The command
	 * \return int : 0 - Success, others - Fail.
	 */
	int cmd(const char *tx) {
		return m_ret = priorSDK.Cmd(m_sessionID, tx, m_rx);
	}

private:
	int m_ret;			    
	char m_rx[1024];
	char m_command[256];

	int m_stageBusy;
	int m_zBusy;

	int m_xSS;
	int m_ySS;
	int m_zSS; // in 10nm

	// speed

	// unit

	PriorScientificSDK priorSDK;
};