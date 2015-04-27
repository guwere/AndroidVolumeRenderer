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
	Shader *m_debugShader;
	static Camera m_camera;
	bool m_constructIntersectRay;
	glm::vec3 m_rayStart, m_rayEnd;
	std::vector<glm::vec3> m_crosshairPts;
	virtual void mainLoop() = 0;
	virtual void setUpdateCallback(void(*updateCallback)(void));
	virtual void handleInput() = 0;
	void loadDebugShader();
	void renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP, bool renderWireframe) const;
	void renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const;
	void renderTextureBasedVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, const TransferFunction &transferFn);

	void drawCrosshair(const glm::vec4 &color) const;

	//void renderRaycastVRmobile1(const Shader *shader, const Mesh &cubeMesh, const Camera &camera, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const
	void(*updateCallback)(void);
	void drawObject(const glm::mat4 &transformMatrix, const std::vector<glm::vec3> &points, GLenum mode, const glm::vec4 &color = glm::vec4(0)) const;
protected:
	Renderer(float screenWidth, float screenHeight);
	Renderer();
	void(*m_updateCallback)(void);
	void getMinMaxPoints(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
	void getMinMaxPoints2(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
	void getMinMaxPoints3(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
};

#endif


