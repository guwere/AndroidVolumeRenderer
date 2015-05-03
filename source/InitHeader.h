#ifndef INITHEADER_H
#define INITHEADER_H
#pragma once

#include "Common.h"
#include "Renderer.h"
#include "Mesh.h"
#include "File.h"
#include "Volume.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "TransferFunction.h"

//#include <omp.h>

Mesh cubeMesh, planeMesh; 
Shader *basicShader, *raycastVRShader, *textureBasedVRShader, *planeShader;
Volume volume;
TransferFunction transferFn;
Renderer *renderer;

//static int r;
void fn(int t, int f, int l, std::vector<int> *r)
{
	for (int i = f; i < l; i++)
	{
		(*r)[30000 - i - 1] = i;
		//LOGI("%d num: %d\n", t, i);
	}
}

//everything that must access the opengl state must come after the window has been initialized (which come as an event in the command queue in the case of android)
void initAppParams()
{


	//Volume volume;
	//volume.parseMHD(File("","CT-Knee.mhd"));
	File basicVS("","basicVS.glsl");
	File basicFS("","basicFS.glsl");
	basicShader = ShaderManager::get().getShader(basicVS, basicFS);
	File raycastVRShaderVS("", "raycastVRShaderVS.glsl");
	File raycastVRShaderFS("", "raycastVRShaderFS.glsl");
	raycastVRShader = ShaderManager::get().getShader(raycastVRShaderVS, raycastVRShaderFS);
	File textureBasedVRShaderVS("", "textureBasedVRShaderVS.glsl");
	File textureBasedVRShaderFS("", "textureBasedVRShaderFS.glsl");
	textureBasedVRShader = ShaderManager::get().getShader(textureBasedVRShaderVS, textureBasedVRShaderFS);
	File planeShaderVS("", "planeVS.glsl");
	File planeShaderFS("", "planeFS.glsl");
	planeShader = ShaderManager::get().getShader(planeShaderVS, planeShaderFS);
	cubeMesh.generateCube(raycastVRShader);
	planeMesh.generatePlane(planeShader);
	cubeMesh.transform.scaleUniform(MESH_SCALE);
	planeMesh.transform.scaleUniform(MESH_SCALE);


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
	renderer->loadDebugShader();

	renderer->loadCudaVolume(volume, transferFn);

	//int *res1 = new int;
	//std::vector<int> res1(30000);
	////*res1 = 0;
	//std::thread t1(fn, 1, 0, 12000, &res1);
	//std::thread t2(fn, 2, 12000, 23000, &res1);
	//std::thread t3(fn, 3, 23000, 30000, &res1);
	//t1.join();
	//t2.join();
	//t3.join();
	//LOGI("res1: %d\n", *res1);
	//int x = res1[0];
}

void updateCallback()
{
	volume.advance();
	//glm::mat4 MVP = camera.GetProjectionMatrix() * camera.GetViewMatrix() * mesh.transform.getMatrix();
	//glm::vec3 cameraPos = glm::vec3(camera.GetViewMatrix()[3]);
	/* Render here */
	//mesh.transform.pivotOnLocalAxis(0, 0.02 * delta, 0);
	//float delta = Timer::get().getLastInterval();
	//mesh.transform.pivotOnLocalAxis(0,0.001, 0);

	int maxRaySteps = MAX_RAY_STEPS;
	float step_size = RAY_STEP_SIZE_MODEL_SPACE;
	//renderer->renderRaycastVR(raycastVRShader, cubeMesh, volume, MAX_RAY_STEPS, RAY_STEP_SIZE_MODEL_SPACE, GRADIENT_STEP_SIZE, LIGHT_POS, transferFn);
	//renderer->renderTextureBasedVR(textureBasedVRShader, cubeMesh, volume, transferFn);
	//renderer->renderTextureBasedVRMT(textureBasedVRShader, cubeMesh, volume, transferFn);
	renderer->renderRaycastVRCUDA(planeShader, planeMesh, volume, MAX_RAY_STEPS, RAY_STEP_SIZE_MODEL_SPACE, GRADIENT_STEP_SIZE, LIGHT_POS, transferFn);
	renderer->renderBasic(basicShader, cubeMesh, renderer->m_camera.GetProjectionMatrix() * renderer->m_camera.GetViewMatrix() * cubeMesh.transform.getMatrix(), true);

	renderer->drawCrosshair(glm::vec4(0,0,1,1));

}

#endif
