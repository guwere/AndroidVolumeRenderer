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

#define MICROSEC_TO_SEC 1.0f/1000000.0f
#define SEC_TO_MICROSEC 1000000.0f

void Timer::updateInterval()
{
	double currTime;
#ifdef WIN32
	currTime = glfwGetTime();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	currTime = MICROSEC_TO_SEC*(double)tv.tv_sec + (double)tv.tv_usec;
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
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	settimeofday(&tv, NULL);
	m_lastTime = 0;
#endif
	m_frameElapsed = 0;
	m_FrameUpdates = 0;
	m_FrameRate = 0.0f;
}

