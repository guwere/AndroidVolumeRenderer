/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "Common.h"
#include "InitHeader.h"
#include "AndroidRenderer.h"

#include "android\asset_manager.h"

AndroidRenderer engine;

void android_main(struct android_app* state) {
   // struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();
	renderer = &engine;
	engine.init(state, initAppParams);
	engine.setUpdateCallback(updateCallback);
	engine.mainLoop();
	//renderer.init(state, initAppParams);

	//Engine &engine = Engine::get();
	//while (1) {
	//	// Read all pending events.
	//	int ident;
	//	int events;
	//	struct android_poll_source* source;

	//	// If not animating, we will block forever waiting for events.
	//	// If animating, we loop until all events are read, then continue
	//	// to draw the next frame of animation.
	//	while ((ident=ALooper_pollAll(engine.m_animating ? 0 : -1, NULL, &events,
	//		(void**)&source)) >= 0) 
	//	{

	//		// Process this event.
	//		if (source != NULL)
	//		{
	//			source->process(engine.m_App, source);
	//		}

	//		// If a sensor has data, process it now.
	//		if (ident == LOOPER_ID_USER)
	//		{
	//			if (engine.m_accelerometerSensor != NULL)
	//			{
	//				ASensorEvent event;
	//				while (ASensorEventQueue_getEvents(engine.m_sensorEventQueue,&event, 1) > 0) {
	//					//LOGI("accelerometer: x=%f y=%f z=%f",event.acceleration.x, event.acceleration.y,event.acceleration.z);
	//				}
	//			}
	//		}

	//		// Check if we are exiting.
	//		if (engine.m_App->destroyRequested != 0) {
	//			engine.termDisplay();
	//			return;
	//		}
	//		glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a);
	//		glClear(CLEAR_MASK);

	//		if(engine.m_animating)
	//			updateCallback();

	//		eglSwapBuffers(engine.m_display, engine.m_surface);
	//	}



	//}

	//android_app_pre_exec_cmd(state, APP_CMD_CONFIG_CHANGED);
    // loop waiting for stuff to do.

	//AAssetDir *asset = AAssetManager_openDir(mgr, "");;




 
}

