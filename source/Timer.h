#ifndef TIMER_H
#define TIMER_H

#pragma once
/**
@brief A class which has a singleton instance serving as a global as but can also be instantiated separately
*/
class Timer
{
public:
	/**Retrieve global instance of the Timer*/
	static Timer& get();
	Timer();

	/**@brief Get the absolute time*/
	double getTime() const;
	/**@brief Retrieve the interval from when updateInteval was last called*/
	double getLastInterval() const;
	/**@brief delta difference curr - last */
	void updateInterval();
	float getFrameRate();
	/**Sets m_LastTime to current time and m_LastInterval to 0*/
	void reset();
private:
	double m_lastTime; //<! the last time (absolute) updateInterval was called
	double m_lastInterval; //<! delta difference curr - last
	double m_frameElapsed;
	int m_FrameUpdates;
	double m_FrameRate;
};
#endif // !TIMER_H
