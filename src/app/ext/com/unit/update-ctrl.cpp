#include "stdafx.h"

#include "update-ctrl.hpp"


updatectrl::updatectrl()
{
	updateInterval = 33;
	state = GAME_STOPPED;
}

shared_ptr<updatectrl> updatectrl::new_instance()
{
	return shared_ptr<updatectrl>(new updatectrl());
}

shared_ptr<updatectrl> updatectrl::getInst()
{
	return shared_from_this();
}

// returns the number of updates to run
int updatectrl::update()
{
	if(state == GAME_RUNNING)
	{
		unsigned long crtTime = pfm::time::get_time_millis();
		unsigned long delta = crtTime - (startTime + pauseTime);
		int ttickCount = delta / updateInterval;

		if(ttickCount > tickCount)
		{
			int ticksLeft = ttickCount - tickCount;
			tickCount = ttickCount;
			return ticksLeft;
		}
	}

	return 0;
}

void updatectrl::started()
{
	state = GAME_RUNNING;
	resetTime();
	//trx("gameStarted");
}

void updatectrl::stopped()
{
	state = GAME_STOPPED;
	tickCount = 0;
	//trx("intr: gameStopped");
}

void updatectrl::paused()
{
	if(state == GAME_RUNNING)
	{
		state = GAME_PAUSED;
		pauseStartTime = pfm::time::get_time_millis();
		//trx("intr: gamePaused");
	}
}

void updatectrl::resumed()
{
	if(state == GAME_PAUSED)
	{
		state = GAME_RUNNING;
		pauseTime += pfm::time::get_time_millis() - pauseStartTime;
		//trx("intr: gameResumed %1%") % pauseTime;
	}
}

int updatectrl::getTickCount()
{
	return tickCount;
}

void updatectrl::setTimeStepDuration(int iupdateInterval)
{
	updateInterval = iupdateInterval;
}

int updatectrl::getTimeStepDuration()
{
	return updateInterval;
}

int updatectrl::getTime()
{
	return tickCount * updateInterval;
}

void updatectrl::resetTime()
{
	tickCount = 0;
	startTime = pfm::time::get_time_millis();
	pauseTime = 0;
	//trx("reset game time");
}
