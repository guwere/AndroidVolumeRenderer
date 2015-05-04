#ifndef TIMER_H
#define TIMER_H

#include "../3rdparty/cudaHelper/helper_timer.h"

#pragma once
/**
@brief A class which has a singleton instance serving as a global as but can also be instantiated separately
*/
class Timer
{
public:
	Timer();
	~Timer();
	StopWatchInterface *m_timer;
	int fpsCount;        // FPS count for averaging
	int fpsLimit;        // FPS limit for sampling
	unsigned int frameCount;
	float ifps;
	float computeFPS();
};
#endif // !TIMER_H
