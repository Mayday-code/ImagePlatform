#include "PriorStage.h"

#include <string>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

void PriorStage::init()
{
	m_rx[0] = 0;

	/* always call Initialise first */
	m_ret = priorSDK.Initialise();// first step

	if (m_ret != PRIOR_OK) {
		cout << "ERROR : in initializing " << m_ret << endl;
		return;
	}
	cout << "Ok initializing " << m_ret << endl;
		
	/* create the session, can be up to 10 */
	m_sessionID = priorSDK.OpenSession();// second step

	if (m_sessionID < 0) {
		cout << "ERROR : in getting sessionId " << m_ret << endl;
		return;
	}
	cout << "sessionId " << m_sessionID << endl;
		
	sprintf_s(m_command, sizeof(m_command), "controller.connect %d", m_port);
	if (cmd(m_command)) {// third step
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	cout << "Ok connecting" << endl;

	m_state = DeviceState::REGISTER;

	if (cmd("controller.z.ss.set 5")) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}
}

void PriorStage::close()
{
	/* disconnect cleanly from controller */
	cmd("controller.disconnect");

	m_ret = priorSDK.CloseSession(m_sessionID);
	cout << "CloseSession " << m_ret << endl;
}

std::pair<double, double> PriorStage::getXYPos()
{
	if (cmd("controller.stage.position.get")) {
		cout << "ERROR : in api" << m_ret << endl;
		return {-3.14, -3.14};
	}

	string position(m_rx);
	int it = std::find(position.cbegin(), position.cend(), ',') - position.cbegin();
	return { std::stod(position.substr(0, it)), std::stod(position.substr(it + 1, position.length())) };
}

double PriorStage::getZPos()
{
	if (cmd("controller.z.position.get")) {
		cout << "ERROR : in api" << m_ret << endl;
		return -3.14;
	}

	std::istringstream iss(m_rx);
	double position;
	iss >> position;
	return position * 10;
}

void PriorStage::moveX(int pos)
{
	sprintf_s(m_command, sizeof(m_command), "controller.stage.goto-position %d %d", pos, 0);

	if (cmd(m_command)) {
		cout << "Api error " << m_ret << endl;
		return;
	}

	//Wait while stage is moving
	do {
		cmd("controller.stage.busy.get");

		m_stageBusy = atoi(m_rx);
	} while (m_stageBusy != 0);
}

void PriorStage::moveY(int pos)
{
	sprintf_s(m_command, sizeof(m_command), "controller.stage.goto-position %d %d", 0, pos);

	if (cmd(m_command)) {
		cout << "Api error " << m_ret << endl;
		return;
	}

	//Wait while stage is moving
	do {
		cmd("controller.stage.busy.get");

		m_stageBusy = atoi(m_rx);
	} while (m_stageBusy != 0);
}

void PriorStage::moveZ(int pos)
{
	sprintf_s(m_command, sizeof(m_command), "controller.z.goto-position %d", pos / 10);

	if (cmd(m_command)) {
		cout << "Api error " << m_ret << endl;
		return;
	}

	//Wait while stage is moving
	do {
		cmd("controller.z.busy.get");

		m_zBusy = atoi(m_rx);
	} while (m_zBusy != 0);
}

void PriorStage::mvrX(bool isForward)
{
	int delta = m_xSS * (isForward ? 1 : -1);
	sprintf_s(m_command, sizeof(m_command), "controller.stage.move-relative %d %d", delta, 0);

	if (cmd(m_command)) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	//Wait while stage is moving
	do {
		cmd("controller.stage.busy.get");

		m_stageBusy = atoi(m_rx);
	} while (m_stageBusy != 0);
}

void PriorStage::mvrY(bool isForward)
{
	int delta = m_ySS * (isForward ? 1 : -1);
	sprintf_s(m_command, sizeof(m_command), "controller.stage.move-relative %d %d", 0, delta);

	if (cmd(m_command)) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	//Wait while stage is moving
	do {
		cmd("controller.stage.busy.get");

		m_stageBusy = atoi(m_rx);
	} while (m_stageBusy != 0);
}

void PriorStage::mvrZ(bool isForward)
{
	int delta = m_zSS * (isForward ? 1 : -1);
	sprintf_s(m_command, sizeof(m_command), "controller.z.move-relative %d", delta);

	if (cmd(m_command)) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	//Wait while stage is moving
	do {
		cmd("controller.z.busy.get");

		m_zBusy = atoi(m_rx);
	} while (m_zBusy != 0);
}