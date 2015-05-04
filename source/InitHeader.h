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
#include "Timer.h"
//#include <omp.h>

Mesh cubeMesh, planeMesh; 
Shader *basicShader, *raycastVRShader, *textureBasedVRShader, *planeShader;
Volume volume;
TransferFunction *transferFn;
TransferFunction transferFn1, transferFn2,transferFn3;
Renderer *renderer;

float currTime, lastTime;
Timer timer;

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


	File vf("",VOLUME_NAME1);
	volume.parseMHD(vf);
	//volume.parsePVM(File("","Box.pvm"));
	//File temp = File("","Box.pvm");
	//temp.print();
	volume.printProperties();
	volume.generate();

	File tf1("",TRANSFER_FN_NAME1_LOW);
	transferFn1.parseVoreenXML(tf1);
	transferFn1.generate();

	File tf2("",TRANSFER_FN_NAME1_MEDIUM);
	transferFn2.parseVoreenXML(tf2);
	transferFn2.generate();

	File tf3("",TRANSFER_FN_NAME1_HIGH);
	transferFn3.parseVoreenXML(tf3);
	transferFn3.generate();


#ifdef CUDA_ENABLED
	renderer->loadCudaVolume(volume, transferFn1);
#endif
	renderer->loadDebugShader();

	sdkCreateTimer(&timer.m_timer);
	sdkStartTimer(&timer.m_timer);
	lastTime = currTime = timer.m_timer->getTime();

}

void updateCallback(unsigned int currRenderType, unsigned int currTransferFn)
{
	switch(currTransferFn)
	{
	case TransferFnType::LOW: transferFn = &transferFn1; break;
	case TransferFnType::MEDIUM: transferFn = &transferFn2; break;
	case TransferFnType::HIGH: transferFn = &transferFn3; break;
	}

	float currTime = timer.m_timer->getTime();
	float delta = currTime - lastTime;
	lastTime = currTime;

	cubeMesh.transform.pivotOnLocalAxis(0,0.00001 * delta, 0);
	planeMesh.transform.pivotOnLocalAxis(0,0.00001 * delta, 0);

	int maxRaySteps = MAX_RAY_STEPS;
	float step_size = RAY_STEP_SIZE_MODEL_SPACE;
	switch (currRenderType)
	{
	case RenderType::RAYTRACE_SHADER:
		renderer->renderRaycastVR(raycastVRShader, cubeMesh, volume, MAX_RAY_STEPS, RAY_STEP_SIZE_MODEL_SPACE, GRADIENT_STEP_SIZE, LIGHT_POS, *transferFn);
		break;
	case RenderType::TEXTURE_BASED:
		renderer->renderTextureBasedVR(textureBasedVRShader, cubeMesh, volume, *transferFn);
		break;
	case RenderType::TEXTURE_BASED_MT:
		renderer->renderTextureBasedVRMT(textureBasedVRShader, cubeMesh, volume, *transferFn);
		break;
#ifdef CUDA_ENABLED
	case RenderType::RAYTRACE_CUDA:
		renderer->renderRaycastVRCUDA(planeShader, planeMesh, volume, MAX_RAY_STEPS, RAY_STEP_SIZE_MODEL_SPACE, GRADIENT_STEP_SIZE);
		break;
#endif
	}
	renderer->renderBasic(basicShader, cubeMesh, renderer->m_camera.GetProjectionMatrix() * renderer->m_camera.GetViewMatrix() * cubeMesh.transform.getMatrix(), true);
	renderer->drawCrosshair(glm::vec4(0,0,1,1));

}

#endif
