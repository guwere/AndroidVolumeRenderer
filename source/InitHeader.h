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

#include <omp.h>

Mesh mesh; 
Shader *basicShader, *raycastVRShader, *textureBasedVRShader;
Volume volume;
TransferFunction transferFn;
Renderer *renderer;



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
	renderer->loadDebugShader();


#pragma omp parallel for 
	for(int i = 0;i < 100;i++) {
		LOGI("num: %d\n", i);
	}

}

void updateCallback()
{
	volume.advance();
	//glm::mat4 MVP = camera.GetProjectionMatrix() * camera.GetViewMatrix() * mesh.transform.getMatrix();
	//glm::vec3 cameraPos = glm::vec3(camera.GetViewMatrix()[3]);
	/* Render here */
	//mesh.transform.pivotOnLocalAxis(0, 0.02 * delta, 0);
	//renderer->renderBasic(basicShader, mesh, renderer->m_camera.GetProjectionMatrix() * renderer->m_camera.GetViewMatrix() * mesh.transform.getMatrix(), true);
	//float delta = Timer::get().getLastInterval();
	//mesh.transform.pivotOnLocalAxis(0,0.001, 0);


	//renderer->renderRaycastVR(raycastVRShader, mesh, volume, MAX_RAY_STEPS, RAY_STEP_SIZE_MODEL_SPACE, GRADIENT_STEP_SIZE, LIGHT_POS, transferFn);
	renderer->renderTextureBasedVR(textureBasedVRShader, mesh, volume, transferFn);

	renderer->drawCrosshair(glm::vec4(0,0,1,1));

}

#endif
