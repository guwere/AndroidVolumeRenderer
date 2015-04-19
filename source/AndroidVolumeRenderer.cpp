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
#include "Engine.h"

#include "File.h"
#include "Volume.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Engine.h"
#include "TransferFunction.h"

#include "android\asset_manager.h"

#pragma region VolumeDeclarations 
Mesh mesh; // gio would be proud of the pragma regions !
Shader *basicShader, *raycastVRShader;
Volume volume;
TransferFunction transferFn;
Engine renderer;
#pragma endregion
void initAppParams();
void updateCallback();

#include <stdint.h>
//everything that must access the opengl state must come after the window has been initialized which come as an event in the command queue
void initAppParams()
{
	//Volume volume;
	//volume.parseMHD(File("","CT-Knee.mhd"));
	File vertexShader("","basicVS.glsl");
	File fragmentShader("","basicFS.glsl");
	basicShader = ShaderManager::get().getShader(vertexShader, fragmentShader);
	File vs("", "raycastVRShaderVS.glsl");
	File fs("", "raycastVRShaderFS.glsl");
	raycastVRShader = ShaderManager::get().getShader(vs, fs);
	mesh.generateCube(raycastVRShader);
	mesh.transform.scaleUniform(MESH_SCALE);


	File vf("",VOLUME_NAME);
	volume.parseMHD(vf);
	//volume.parsePVM(File("","Box.pvm"));
	//File temp = File("","Box.pvm");
	//temp.print();
	volume.printProperties();
	volume.generate();

	File tf("",TRANSFER_FN_NAME);
	transferFn.parseVoreenXML(tf);
	transferFn.generate();


}

void updateCallback()
{
	volume.advance();
	glm::mat4 proj = renderer.m_camera.GetProjectionMatrix();
	glm::mat4 view = renderer.m_camera.GetViewMatrix();
	glm::mat4 model = glm::mat4();
	glm::mat4 MVP = proj * view * model;
	renderer.renderBasic(basicShader, mesh, MVP);
	//renderer.renderRaycastVR(raycastVRShader, mesh, volume, MAX_RAY_STEPS, RAY_STEP_SIZE, GRADIENT_STEP_SIZE, LIGHT_POS, transferFn);

}

void android_main(struct android_app* state) {
   // struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();
	renderer.init(state, initAppParams);
	renderer.setUpdateCallback(updateCallback);
	renderer.mainLoop();
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
//END_INCLUDE(all)
