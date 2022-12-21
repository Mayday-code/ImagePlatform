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
	m_ret = priorSDK.Initialise();//-first step

	if (m_ret != PRIOR_OK) {
		cout << "ERROR : in initialising " << m_ret << endl;
		return;
	}
	cout << "Ok initialising " << m_ret << endl;
		
	/* create the session, can be up to 10 */
	m_sessionID = priorSDK.OpenSession();//-second step

	if (m_sessionID < 0) {
		cout << "ERROR : in getting sessionId " << m_ret << endl;
		return;
	}
	cout << "sessionId " << m_sessionID << endl;
		
	sprintf_s(m_command, sizeof(m_command), "controller.connect ");
	char src[3];
	sprintf_s(src, sizeof(src), "%d", m_port);
	strcat_s(m_command, sizeof(m_command), src);
	if (cmd(m_command)) {//-third step
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	cout << "Ok connecting" << endl;

	m_state = DeviceState::REGISTER;

	if (cmd("controller.z.ss.set 10")) {
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
	return { std::atof(position.substr(0, it).c_str()), std::atof(position.substr(it + 1, position.length()).c_str()) };
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
	return position / 100;
}

//void PriorStage::m_moveXY()
//{
//	//sprintf_s(m_command, sizeof(m_command), "controller.stage.goto-position ");
//	//char src[20];
//	//sprintf_s(src, sizeof(src), "%d %d", static_cast<int>(target_value[0]), static_cast<int>(target_value[1]));
//	//strcat_s(command, sizeof(command), src);
//
//	//if (cmd(command)) {
//	//	cout << "Api error " << m_ret << endl;
//	//	return;
//	//}
//
//	////移动之后等待；
//	//do {
//	//	cmd("controller.stage.busy.get");
//
//	//	m_stageBusy = atoi(m_rx);
//	//} while (m_stageBusy != 0);
//}

//void PriorStage::m_moveZ()
//{
//	//sprintf_s(m_command, sizeof(command), "controller.z.goto-position ");
//	//char src[10];
//	//sprintf_s(src, sizeof(src), "%d", static_cast<int>(target_value[2]));
//	//strcat_s(command, sizeof(command), src);
//
//	//if (cmd(command)) {
//	//	cout << "Api error " << ret << endl;
//	//	return;
//	//}
//
//	////移动之后等待；
//	//do {
//	//	cmd("controller.z.busy.get");
//
//	//	zBusy = atoi(rx);
//	//} while (zBusy != 0);
//}

void PriorStage::mvrX(bool isForward)
{
	int delta = m_xSS * (isForward ? 1 : -1);
	sprintf_s(m_command, sizeof(m_command), "controller.stage.move-relative ");
	char src[20];
	sprintf_s(src, sizeof(src), "%d %d", delta, 0);
	strcat_s(m_command, sizeof(m_command), src);

	if (cmd(m_command)) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	//移动之后等待；
	do {
		cmd("controller.stage.busy.get");

		m_stageBusy = atoi(m_rx);
	} while (m_stageBusy != 0);

	m_x += delta;
}

void PriorStage::mvrY(bool isForward)
{
	int delta = m_ySS * (isForward ? 1 : -1);
	sprintf_s(m_command, sizeof(m_command), "controller.stage.move-relative ");
	char src[20];
	sprintf_s(src, sizeof(src), "%d %d", 0, delta);
	strcat_s(m_command, sizeof(m_command), src);

	if (cmd(m_command)) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	//移动之后等待；
	do {
		cmd("controller.stage.busy.get");

		m_stageBusy = atoi(m_rx);
	} while (m_stageBusy != 0);

	m_y += delta;
}

//测试Z轴方向
void PriorStage::mvrZ(bool isForward)
{
	int delta = m_zSS * (isForward ? 1 : -1);
	sprintf_s(m_command, sizeof(m_command), "controller.z.move-relative ");
	char src[10];
	sprintf_s(src, sizeof(src), "%d", delta);
	strcat_s(m_command, sizeof(m_command), src);

	if (cmd(m_command)) {
		cout << "ERROR : in api" << m_ret << endl;
		return;
	}

	//移动之后等待；
	do {
		cmd("controller.z.busy.get");

		m_zBusy = atoi(m_rx);
	} while (m_zBusy != 0);

	m_z += delta;
}