#pragma once

#include "MyDefine.h"
#include <utility>

/*!
 * \class Stage
 *
 * \brief Base class for all stages. To use Stage, first construct it, and then init it.
 *
 * \author XYH
 * \date 12 2022
 */
class Stage {
public:
	Stage(int t_port) : m_port(t_port) { }
	virtual ~Stage() = default;

	virtual void init() = 0;

	/*!
	 * \brief Request X and Y to move to the given position
	 */
	virtual void moveXY(int xpos, int ypos) = 0;

	/*!
	 * \brief Request the Z to move to the given position
	 * \param pos Z-target position in nm
	 */
	virtual void moveZ(int pos) = 0;

	/*!
	 * \brief Request the stage to move along the X axis relative to its current position.
	 * \param direction The moving direction. True means right, false means left.
	 */
	virtual void mvrX(bool direction) = 0;

	/*!
	 * \brief Request the stage to move along the Y axis relative to its current position.
	 * \param direction The moving direction. True means far away from stage, false means close to stage.
	 */
	virtual void mvrY(bool direction) = 0;

	/*!
	 * \brief Request the stage to move along the Z axis relative to its current position.
	 * \param direction The moving direction.
	 */
	virtual void mvrZ(bool direction) = 0;


	virtual std::pair<double, double> getXYPos() = 0;

	/*!
	 * \brief Request the coordinate which can be directly printed in UI.
	 */
	virtual double getZPos() = 0;

	int getID() { return m_sessionID; }
	void setID(int t_ID) { m_sessionID = t_ID; }

	/*!
	 * \brief Set the step size in X direction
	 * \param t_XSS The step size you want to set.
	 */
	virtual void setXSS(int t_XSS) = 0;

	/*!
	 * \brief Set the step size in Y direction
	 * \param t_YSS The step size you want to set.
	 */
	virtual void setYSS(int t_YSS) = 0;

	/*!
	 * \brief Set the step size in Z direction
	 * \param t_ZSS The step size you want to set - in 10nm
	 */
	virtual void setZSS(int t_ZSS) = 0;

	DeviceState getState() const { return m_state; }
protected:
	DeviceState m_state = DeviceState::NOTREGISTER;

	int m_sessionID;	//ID of this connection

	int m_port;
};