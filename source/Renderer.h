#ifndef RENDERER_H
#define RENDERER_H
#pragma once

#include "Common.h"
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Volume.h"
#include "Shader.h"
#include "TransferFunction.h"

#include <GLFW/glfw3.h>

class Renderer
{
public:
	glm::vec4 m_clearColor;
	GLbitfield m_clearMask;

	Renderer();
	Renderer(const glm::vec4 &clearColor, const GLbitfield &clearMask);
	void preRenderUpdate();
	void postRenderUpdate(GLFWwindow* window);
	void renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP) const;
	void renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const glm::mat4 MVP, const Volume &volume, const glm::vec3 &eyePos, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPos, const TransferFunction &transferFn) const;
private:
};
#endif


