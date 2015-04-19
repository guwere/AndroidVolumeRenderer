#include "EGLRenderer.h"
#include "Timer.h"


EGLRenderer::EGLRenderer()
{

}

void EGLRenderer::init(struct android_app* state, void (*applicationInitCallback)(void))
{


}

void EGLRenderer::mainLoop(void(*updateCallback)(void))
{
	Engine &engine = Engine::get();
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident=ALooper_pollAll(engine.m_animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) 
		{

			// Process this event.
			if (source != NULL)
			{
				source->process(engine.m_App, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER)
			{
				if (engine.m_accelerometerSensor != NULL)
				{
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.m_sensorEventQueue,&event, 1) > 0) {
						//LOGI("accelerometer: x=%f y=%f z=%f",event.acceleration.x, event.acceleration.y,event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (engine.m_App->destroyRequested != 0) {
				engine.termDisplay();
				return;
			}
			glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
			glClear(m_clearMask);

			if(engine.m_animating)
				updateCallback();

			eglSwapBuffers(engine.m_display, engine.m_surface);
		}
			


	}
	
}


void EGLRenderer::handleInput()
{

}


