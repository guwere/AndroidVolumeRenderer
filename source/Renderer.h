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

#include <cuda.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "../3rdparty/cudaHelper/helper_cuda.h"
#include "../3rdparty/cudaHelper/helper_math.h"

class Renderer
{
public:
	glm::vec4 m_clearColor;
	GLbitfield m_clearMask;
	int m_screenWidth, m_screenHeight;
	float m_aspectRatio;
	dim3 m_gridSize, m_blockSize;
	Shader *m_debugShader;
	static Camera m_camera;
	bool m_constructIntersectRay;
	glm::vec3 m_rayStart, m_rayEnd;

	struct ThreadParameters
	{
		ThreadParameters(int threadId, glm::vec3 &maxPos,  glm::vec3 &dirSample,  const Mesh &cubeMesh,  std::vector<Edge> &transformedEdges, int first, int last, std::vector<PTVEC3> &sortedProxyPlanes);
		int threadId;
		glm::vec3 &maxPos;
		glm::vec3 &dirSample;
		const Mesh &cubeMesh;
		std::vector<Edge> &transformedEdges;
		int first;
		int last;
		std::vector<PTVEC3> &sortedProxyPlanes;
	};

	PTVEC3 m_crosshairPts;

	GLuint m_cudaPBO;
	GLuint m_cudaTex;
	struct cudaGraphicsResource *cuda_pbo_resource; // CUDA Graphics Resource (to transfer PBO)

	virtual void mainLoop() = 0;
	virtual void setUpdateCallback(void(*updateCallback)(void));
	virtual void handleInput() = 0;
	void loadDebugShader();
	void loadCudaVolume(const Volume &volume, const TransferFunction &transferFunction);
	void renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP, bool renderWireframe) const;
	void renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const;
	void renderTextureBasedVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, const TransferFunction &transferFn);
	void renderTextureBasedVRMT(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, const TransferFunction &transferFn);
	void renderRaycastVRCUDA(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn);

	void calculateProxyPlanes(ThreadParameters &params);
	void sortPolygonClockwise(const PTVEC3 &proxyPlane, glm::vec3 centerPt, PTVEC3 &sortedProxyPlane) const;
	void getClosestPtsOnEdges(const glm::vec3 &maxPos, const glm::vec3 &dirSample, int currSample, const Mesh &cubeMesh, const std::vector<Edge> &transformedEdges, PTVEC3 &proxyPlane, glm::vec3 &centerPt) const;

	void drawCrosshair(const glm::vec4 &color) const;

	//void renderRaycastVRmobile1(const Shader *shader, const Mesh &cubeMesh, const Camera &camera, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const
	void(*updateCallback)(void);
	void drawObject(const glm::mat4 &transformMatrix, const std::vector<glm::vec3> &points, GLenum mode, const glm::vec4 &color = glm::vec4(0)) const;
protected:
	//Renderer(float screenWidth, float screenHeight);
	//Renderer();
	void init(float screenWidth, float screenHeight);
	~Renderer();
	void initPixelBufferCuda();
	void(*m_updateCallback)(void);
	void getMinMaxPointsVertices(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
	void getMinMaxPointsCube(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;
	void getMinMaxPointsView(const std::vector<glm::vec3> positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const;

	void writeUniform(GLuint shaderId, const char *uniformName, float val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, int val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::vec3 val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::vec4 val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::mat3 val) const;
	void writeUniform(GLuint shaderId, const char *uniformName, glm::mat4 val) const;
	void writeUniform2DTex(GLuint shaderId, const char *uniformName, unsigned int texUnit, GLuint texId) const;
	void writeUniform3DTex(GLuint shaderId, const char *uniformName, unsigned int texUnit, GLuint texId) const;

};

extern "C" void initCuda();
extern "C" void initCudaVolume(void *volume, cudaExtent volumeSize, GLfloat *transferFunction, int transferFunctionSize);
extern "C" void exitCuda();
extern "C" void render_kernel(dim3 gridSize, dim3 blockSize, uint *d_output, uint imageW, uint imageH, float *invViewMatrix, float aspectRatio, float maxRaySteps, float rayStepSize);
extern "C" void copyInvViewMatrix(float *invViewMatrix, size_t sizeofMatrix);
extern "C" void copyRay(float *rayOrigin, float *rayDirection);

#endif


