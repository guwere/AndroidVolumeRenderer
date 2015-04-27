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
	PVEC3 m_crosshairPts;
	virtual void mainLoop() = 0;
	virtual void setUpdateCallback(void(*updateCallback)(void));
	virtual void handleInput() = 0;
	void loadDebugShader();
	void renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP, bool renderWireframe) const;
	void renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const;
	void renderTextureBasedVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, const TransferFunction &transferFn);

	void sortPolygonClockwise(const PVEC3 &proxyPlane, glm::vec3 centerPt, PVEC3 &sortedProxyPlane) const;

	void getClosestPtsOnEdges(const glm::vec3 &maxPos, const glm::vec3 &dirSample, int currSample, const Mesh &cubeMesh, const std::vector<Edge> &transformedEdges, PVEC3 &proxyPlane, glm::vec3 &centerPt) const;

	void drawCrosshair(const glm::vec4 &color) const;

	//void renderRaycastVRmobile1(const Shader *shader, const Mesh &cubeMesh, const Camera &camera, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const
	void(*updateCallback)(void);
	void drawObject(const glm::mat4 &transformMatrix, const std::vector<glm::vec3> &points, GLenum mode, const glm::vec4 &color = glm::vec4(0)) const;
protected:
	Renderer(float screenWidth, float screenHeight);
	Renderer();
	void(*m_updateCallback)(void);
	void getMinMaxPointsVertices(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
	void getMinMaxPointsCube(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
	void getMinMaxPointsView(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;

	void writeUniform(GLuint shaderId, const char *uniformName, float val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, int val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::vec3 val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::mat3 val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::mat4 val) const;
	void writeUniform2DTex(GLuint shaderId, const char *uniformName, unsigned int texUnit, GLuint texId) const;
	void writeUniform3DTex(GLuint shaderId, const char *uniformName, unsigned int texUnit, GLuint texId) const;

};

#endif


