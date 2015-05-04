#include "Timer.h"


Timer::Timer()
{
	fpsCount = 0;        // FPS count for averaging
	fpsLimit = 1;        // FPS limit for sampling
	frameCount = 0;
	sdkCreateTimer(&m_timer);
}

Timer::~Timer()
{
	sdkDeleteTimer(&m_timer);
}
#ifndef MAX
#define MAX(a,b) ((a > b) ? a : b)
#endif

float Timer::computeFPS()
{
	frameCount++;
	fpsCount++;

	if (fpsCount == fpsLimit)
	{
		char fps[256];
		ifps = 1.f / (sdkGetAverageTimerValue(&m_timer) / 1000.f);
		//sprintf(fps, "Volume Render: %3.1f fps", ifps);

		//glutSetWindowTitle(fps);
		fpsCount = 0;

		fpsLimit = (int)MAX(1.f, ifps);
		sdkResetTimer(&m_timer);
	}
	return ifps;
}
