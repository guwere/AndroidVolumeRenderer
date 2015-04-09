#include "Timer.h"
#include "Common.h"

Timer& Timer::get()
{
	static Timer singleton;
	return singleton;
}

Timer::Timer()
{
	reset();
}

double Timer::getTime() const
{
	return m_lastTime; 
}

double Timer::getLastInterval() const
{
	return m_lastInterval;
}
#define MICROSEC_TO_SEC 1.0f/1000000.0f;
void Timer::updateInterval()
{
	double currTime;
#ifdef WIN32
	currTime = glfwGetTime();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec + tv.tv_usec * MICROSEC_TO_SEC;
#endif
	m_lastInterval = currTime - m_lastTime;
	m_lastTime = currTime;
	m_frameElapsed += m_lastInterval;
	m_FrameUpdates++;

	if(m_frameElapsed >= 1.0f) // one second
	{
		m_frameElapsed = 0;
		m_FrameRate = m_FrameUpdates;
		m_FrameUpdates = 0;
	}
}
float Timer::getFrameRate()
{
	return m_FrameRate;
}
void Timer::reset()
{
	m_lastInterval = 0;
#ifdef WIN32
	m_lastTime = glfwGetTime();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	m_lastTime = tv.tv_sec + tv.tv_usec * MICROSEC_TO_SEC;
#endif
	m_frameElapsed = 0;
	m_FrameUpdates = 0;
	m_FrameRate = 0.0f;
}

