#ifndef RENDERER_H
#define RENDERER_H
#pragma once

#include "Common.h"
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Volume.h"
#include "Shader.h"
#include "Camera.h"
#include "TransferFunction.h"


class Renderer
{
public:
	glm::vec4 m_clearColor;
	GLbitfield m_clearMask;
	int m_screenWidth, m_screenHeight;
	static Camera m_camera;
	virtual void mainLoop(void(*updateCallback)(void)) = 0;
	virtual void handleInput() = 0;
	void renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP) const;
	void renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const;
	//void renderRaycastVRmobile1(const Shader *shader, const Mesh &cubeMesh, const Camera &camera, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const

protected:
	Renderer(float screenWidth, float screenHeight);

};

#endif


