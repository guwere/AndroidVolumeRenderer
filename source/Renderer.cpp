﻿#include "Renderer.h"
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/intersect.hpp>
#include "HelperFunctions.h"
#include "ShaderManager.h"

#ifdef CUDA_ENABLED
#include <cuda_gl_interop.h>
#endif


Camera Renderer::m_camera;


//Renderer::Renderer(float screenWidth, float screenHeight)
//	:m_screenWidth(screenWidth), m_screenHeight(screenHeight)
//{
//
//}

void Renderer::init(float screenWidth, float screenHeight)
{
	m_currRenderType = RenderType::RAYTRACE_SHADER;
	m_availableRenderTypes.push_back(RAYTRACE_SHADER);
	m_availableRenderTypes.push_back(TEXTURE_BASED);
	m_availableRenderTypes.push_back(TEXTURE_BASED_MT);
#ifdef CUDA_ENABLED
	m_availableRenderTypes.push_back(RAYTRACE_CUDA);
#endif
	m_currTransferFnType =TransferFnType::LOW;
	m_availableTransferFnType.push_back(LOW);
	m_availableTransferFnType.push_back(MEDIUM);
	m_availableTransferFnType.push_back(HIGH);

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_aspectRatio = screenWidth / screenHeight;
	m_clearColor = CLEAR_COLOR;
	m_clearMask = CLEAR_MASK;
	m_constructIntersectRay = false;
	//m_camera.setPosition(INITIAL_CAMERA_POS);
	m_camera.projection = glm::perspective(CAMERA_FOV, screenWidth / screenHeight, NEAR_CLIP_PLANE, FAR_CLIP_PLANE);
	m_crosshairPts.push_back(glm::vec3(-0.03f,0,0));
	m_crosshairPts.push_back(glm::vec3(0.03f,0,0));
	m_crosshairPts.push_back(glm::vec3(0,-0.03f,0));
	m_crosshairPts.push_back(glm::vec3(0,0.03f,0));
#ifdef CUDA_ENABLED
	m_blockSize = dim3(BLOCK_SIZE, BLOCK_SIZE);
	m_gridSize = dim3(HelperFunctions::iDivUp(m_screenWidth, m_blockSize.x), HelperFunctions::iDivUp(m_screenHeight, m_blockSize.y));
	m_cudaPBO = 0;
	cuda_pbo_resource = NULL;
	initCuda();
	initPixelBufferCuda();
#endif
}


Renderer::~Renderer()
{
#ifdef CUDA_ENABLED
	exitCuda();
#endif
}
#ifdef CUDA_ENABLED
void Renderer::initPixelBufferCuda()
{

	if (m_cudaPBO)
	{
		// unregister this buffer object from CUDA C
		checkCudaErrorsLog(cudaGraphicsUnregisterResource(cuda_pbo_resource));

		// delete old buffer
		glDeleteBuffers(1, &m_cudaPBO);
		glDeleteTextures(1, &m_cudaTex);
	}

	// create pixel buffer object for display
	glGenBuffers(1, &m_cudaPBO);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_cudaPBO);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, m_screenWidth*m_screenHeight*sizeof(GLubyte)*4, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// register this buffer object with CUDA
	checkCudaErrorsLog(cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, m_cudaPBO, cudaGraphicsMapFlagsWriteDiscard));

	// create texture for display
	glGenTextures(1, &m_cudaTex);
	glBindTexture(GL_TEXTURE_2D, m_cudaTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_screenWidth, m_screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#endif
void Renderer::getMinMaxPointsVertices(const PTVEC3 positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const
{
	//assume first point as both min and max
	float currMin, currMax, distanceToCamera ;
	currMin = currMax = positions.at(0).z;
	int currMinIndex = 0, currMaxIndex = 0;
	for (unsigned int i = 1; i < cubeMesh.m_Vertices.size(); ++i)
	{
		const glm::vec3 &vertexView = positions[i];
		//distanceToCamera = abs(vertexView.z);
		distanceToCamera = vertexView.z;
		if(distanceToCamera < currMin)
		{
			currMin = distanceToCamera;
			currMinIndex = i;
		}
		else if(distanceToCamera > currMax)
		{
			currMax = distanceToCamera;
			currMaxIndex = i;
		}
	}
	//maxPos = glm::vec3(0, 0, positions.at(currMinIndex).z);
	//minPos = glm::vec3(0, 0, positions.at(currMaxIndex).z);	
	maxPos = positions.at(currMinIndex);
	minPos = positions.at(currMaxIndex);

}

void Renderer::getMinMaxPointsCube(const PTVEC3 positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const
{
	PTVEC3 intersectioPts;
	glm::vec3 origin = glm::vec3(0);
	glm::vec3 direction  = glm::vec3(0,0,-1);
	for (unsigned int i = 0; i < cubeMesh.m_Indices.size(); i +=3)
	{

		glm::vec3 v1 = positions[cubeMesh.m_Indices[i]];
		glm::vec3 v2 = positions[cubeMesh.m_Indices[i+1]];
		glm::vec3 v3 = positions[cubeMesh.m_Indices[i+2]];

		glm::vec3 uvtCoord;
		//if(glm::intersectRayTriangle(origin, direction , vertexView1, vertexView2, vertexView3, intersectPos))
		if(HelperFunctions::intersectRayTriangle(v1, v2, v3, true, origin, direction , uvtCoord))
		{
			glm::vec3 d1 = (v2 - v1) * uvtCoord.x;
			glm::vec3 d2 = (v3 - v1) * uvtCoord.y;
			glm::vec3 intersecPt = v1 + (d1 + d2); 
			intersectioPts.push_back(intersecPt);
		}
	}
	if(intersectioPts.size() < 2)
		return;

	float distance1 = glm::length2(intersectioPts.at(0));
	float distance2 = glm::length2(intersectioPts.at(1));
	if(distance1 < distance2)
	{
		minPos = intersectioPts.at(0);
		maxPos = intersectioPts.at(1);
	}
	else
	{
		minPos = intersectioPts.at(1);
		maxPos = intersectioPts.at(0);
	}

}

void Renderer::getMinMaxPointsView(const PTVEC3 positions, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const
{
	//assume first point as both min and max
	float currMin, currMax, distanceToCamera ;
	currMin = currMax = positions.at(0).z;
	int currMinIndex = 0, currMaxIndex = 0;
	for (unsigned int i = 1; i < cubeMesh.m_Vertices.size(); ++i)
	{
		const glm::vec3 &vertexView = positions[i];
		//distanceToCamera = abs(vertexView.z);
		distanceToCamera = vertexView.z;
		if(distanceToCamera < currMin)
		{
			currMin = distanceToCamera;
			currMinIndex = i;
		}
		else if(distanceToCamera > currMax)
		{
			currMax = distanceToCamera;
			currMaxIndex = i;
		}
	}
	maxPos = glm::vec3(0, 0, positions.at(currMinIndex).z);
	minPos = glm::vec3(0, 0, positions.at(currMaxIndex).z);	
	//maxPos = positions.at(currMinIndex);
	//minPos = positions.at(currMaxIndex);

}

void Renderer::loadDebugShader()
{
	File vs("", std::string(DEBUG_VERTEX_SHADER_NAME));
	File fs("", std::string(DEBUG_FRAGMENT_SHADER_NAME));
	m_debugShader = ShaderManager::get().getShader(vs,fs);
}

#ifdef CUDA_ENABLED
void Renderer::loadCudaVolume(const Volume &volume, const TransferFunction &transferFunction)
{
	void *volumePtr = volume.m_memblock3D;
	cudaExtent extent;
	extent.width = volume.m_xRes;
	extent.height = volume.m_yRes;
	extent.depth = volume.m_zRes;

	GLfloat *transferFnPtr = (GLfloat*)&transferFunction.m_colorTable[0];
	initCudaVolume(volumePtr, extent, transferFnPtr, TRANSFER_FN_TABLE_SIZE);
}
#endif // CUDA_ENABLED

void Renderer::setUpdateCallback(void(*updateCallback)(unsigned int, unsigned int))
{
	this->updateCallback = updateCallback;
}

void Renderer::writeUniform(GLuint shaderId, const char *uniformName, float val) const
{
	GLuint uniformLoc = glGetUniformLocation (shaderId, uniformName);
	glUniform1f (uniformLoc, val);
}

void Renderer::writeUniform(GLuint shaderId, const char *uniformName, int val) const
{
	GLuint uniformLoc = glGetUniformLocation (shaderId, uniformName);
	glUniform1i (uniformLoc, val);
}

void Renderer::writeUniform(GLuint shaderId, const char *uniformName, glm::vec3 val) const
{
	GLuint uniformLoc = glGetUniformLocation (shaderId, uniformName);
	glUniform3fv(uniformLoc, 1, glm::value_ptr(val));
}

void Renderer::writeUniform(GLuint shaderId, const char *uniformName, glm::vec4 val) const
{
	GLuint uniformLoc = glGetUniformLocation (shaderId, uniformName);
	glUniform4fv(uniformLoc, 1, glm::value_ptr(val));
}

void Renderer::writeUniform(GLuint shaderId, const char *uniformName, glm::mat3 val) const
{
	GLuint uniformLoc = glGetUniformLocation (shaderId, uniformName);
	glUniformMatrix3fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(val));
}

void Renderer::writeUniform(GLuint shaderId, const char *uniformName, glm::mat4 val) const
{
	GLuint uniformLoc = glGetUniformLocation (shaderId, uniformName);
	glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(val));
}

void Renderer::writeUniform2DTex(GLuint shaderId, const char *uniformName, unsigned int texUnit, GLuint texId) const
{
	glActiveTexture (GL_TEXTURE0 + texUnit);
	GLuint uniformLoc = glGetUniformLocation(shaderId, uniformName);
	glUniform1i(uniformLoc, texUnit);
	glBindTexture (GL_TEXTURE_2D, texId);
}

void Renderer::writeUniform3DTex(GLuint shaderId, const char *uniformName, unsigned int texUnit, GLuint texId) const
{
	glActiveTexture (GL_TEXTURE0 + texUnit);
	GLuint uniformLoc = glGetUniformLocation(shaderId, uniformName);
	glUniform1i(uniformLoc, texUnit);
	glBindTexture (GL_TEXTURE_3D, texId);
}

void Renderer::renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP, bool renderWireframe) const
{
	glEnable(GL_DEPTH_TEST);
	glUseProgram(shader->getId());
	glUniformMatrix4fv(glGetUniformLocation(shader->getId(),"MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	if(renderWireframe)
		mesh.renderWireframe();
	else
		mesh.render();
	glDisable(GL_DEPTH_TEST);

}

void Renderer::renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const
{
	glEnable(GL_DEPTH_TEST);
	const glm::mat4 MV = m_camera.GetViewMatrix() * cubeMesh.transform.getMatrix();
	const glm::mat4 MVP = m_camera.GetProjectionMatrix() * MV;
	const glm::mat4 inverseM = glm::inverse(cubeMesh.transform.getMatrix());
	const glm::vec3 lightPos =  TRANSFORM3(inverseM, lightPosWorld);
	const glm::vec3 camPosModel =  TRANSFORM3(inverseM,  m_camera.getPosition());

	GLuint shaderId = shader->getId();
	glUseProgram(shader->m_Id);
	writeUniform(shaderId, "MVP", MVP);
	writeUniform3DTex(shaderId, "volume", 0, volume.getTextureId());
	writeUniform(shaderId, "camPos", camPosModel);
	writeUniform(shaderId, "maxRaySteps", (int)maxRaySteps);
	writeUniform(shaderId, "rayStepSize", rayStepSize);
	writeUniform(shaderId, "gradientStepSize", gradientStepSize);
	writeUniform(shaderId, "lightPosition", lightPos);
	writeUniform3DTex(shaderId, "volume", 0, volume.getTextureId());
	writeUniform2DTex(shaderId, "transferFunc", 1, transferFn.getTextureId());

	cubeMesh.render(GL_TRIANGLES, true);
}

void Renderer::renderTextureBasedVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, const TransferFunction &transferFn)
{
	const glm::mat4 &modelMatrix = cubeMesh.transform.getMatrix();
	const glm::mat4 &viewMatrix = m_camera.GetViewMatrix();
	const glm::mat4 &projMatrix = m_camera.GetProjectionMatrix();
	glm::mat4 modelViewMatrix = viewMatrix * cubeMesh.transform.getMatrix();
	glm::mat4 inverseMV = glm::inverse(modelViewMatrix);

	PTVEC3 transformedPositions = cubeMesh.getTransformedPositions(modelViewMatrix);
	std::vector<Edge> transformedEdges = cubeMesh.getTransformedEdges(modelViewMatrix);

	//enable proper alpha blending for back-to-front order
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	GLuint shaderId = shader->getId();
	glUseProgram(shaderId);
	//write uniform variables
	GLuint uniformLoc;
	writeUniform3DTex(shaderId, "volume", 0, volume.getTextureId());
	writeUniform2DTex(shaderId, "transferFunc", 1, transferFn.getTextureId());
	writeUniform(shaderId, "projMatrix", projMatrix);
	writeUniform(shaderId, "inverseMV", inverseMV);

	glm::vec3 minPos, maxPos;
	//Find the minimum and maximum z coordinates of transformed vertices in view space
	getMinMaxPointsCube(transformedPositions, cubeMesh, minPos, maxPos);

	//Compute the number of sampling planes used between these two values using equidistant spacing from the view origin
	glm::vec3 backToFrontVec = minPos-maxPos;
	float lengthTotal = glm::length(backToFrontVec);
	glm::vec3 dirModel = glm::vec3(inverseMV * glm::vec4(minPos, 1.0f)) - glm::vec3(inverseMV * glm::vec4(maxPos, 1.0f));
	float lengthTotalModel = glm::length(dirModel);
	backToFrontVec = glm::normalize(backToFrontVec);
	glm::vec3 dirSample = backToFrontVec * float(RAY_STEP_SIZE_MODEL_SPACE) * (MESH_SCALE);
//	For each plane in front-to-back or back-to-front order
	const int maxRays = MAX_RAY_STEPS * (lengthTotalModel / MESH_CUBE_DIAGONAL_LEN); // dependent on the ratio of maximum allowed rays and the current direction

	for (int currSample = 1; currSample < (int)maxRays; ++currSample)
	{
		
		//Test for intersections with the edges of the bounding box. 
		//Add each intersection point to a temporary vertex list.
		//Up to six intersections are generated, so the maximum size of the list is fixed.
		glm::vec3 centerPt;
		PTVEC3 proxyPlane;
		getClosestPtsOnEdges(maxPos, dirSample, currSample, cubeMesh, transformedEdges, proxyPlane, centerPt);
	//	//Compute the center of the proxy polygon by averaging the intersection points.
		//Sort the polygon vertices clockwise or counterclockwise by projecting them onto the x-y plane
		//and computing their angle around the center, with the first vertex or the x axis as the reference.
		PTVEC3 sortedProxyPlane;
		sortPolygonClockwise(proxyPlane, centerPt, sortedProxyPlane);

		//Tessellate the proxy polygon into triangles and add the resulting vertices to the output vertex array. 
		//The slice polygon can be tessellated into a triangle strip or a triangle fan using the center.
		//Depending on the rendering algorithm, the vertices may need to be transformed back to object space during this step.
		GLuint proxyVBO;
		glGenBuffers(1, &proxyVBO);
		glBindBuffer(GL_ARRAY_BUFFER, proxyVBO);
		glBufferData(GL_ARRAY_BUFFER, sortedProxyPlane.size() * sizeof(glm::vec3), &sortedProxyPlane[0], GL_DYNAMIC_DRAW);
		GLuint position = glGetAttribLocation(shader->m_Id,"position");
		glEnableVertexAttribArray(position);
		glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glDrawArrays(GL_TRIANGLE_FAN,0, sortedProxyPlane.size());
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glDeleteBuffers(1, &proxyVBO);

	}

}

void Renderer::renderTextureBasedVRMT(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, const TransferFunction &transferFn)
{
	const glm::mat4 &modelMatrix = cubeMesh.transform.getMatrix();
	const glm::mat4 &viewMatrix = m_camera.GetViewMatrix();
	const glm::mat4 &projMatrix = m_camera.GetProjectionMatrix();
	const glm::mat4 modelViewMatrix = viewMatrix * cubeMesh.transform.getMatrix();
	const glm::mat4 inverseMV = glm::inverse(modelViewMatrix);

	PTVEC3 transformedPositions = cubeMesh.getTransformedPositions(modelViewMatrix);
	std::vector<Edge> transformedEdges = cubeMesh.getTransformedEdges(modelViewMatrix);

	//enable proper alpha blending for back-to-front order
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glBlendEquation(GL_ADD);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	GLuint shaderId = shader->getId();
	glUseProgram(shaderId);
	//write uniform variables
	GLuint uniformLoc;
	writeUniform3DTex(shaderId, "volume", 0, volume.getTextureId());
	writeUniform2DTex(shaderId, "transferFunc", 1, transferFn.getTextureId());
	writeUniform(shaderId, "projMatrix", projMatrix);
	writeUniform(shaderId, "inverseMV", inverseMV);

	glm::vec3 minPos, maxPos;
	//Find the minimum and maximum z coordinates of transformed vertices in view space
	getMinMaxPointsCube(transformedPositions, cubeMesh, minPos, maxPos);

	//Compute the number of sampling planes used between these two values using equidistant spacing from the view origin
	glm::vec3 backToFrontVec = minPos-maxPos;
	float lengthTotal = glm::length(backToFrontVec);
	glm::vec3 dirModel = glm::vec3(inverseMV * glm::vec4(minPos, 1.0f)) - glm::vec3(inverseMV * glm::vec4(maxPos, 1.0f));
	float lengthTotalModel = glm::length(dirModel);
	backToFrontVec = glm::normalize(backToFrontVec);
	glm::vec3 dirSample = backToFrontVec * float(RAY_STEP_SIZE_MODEL_SPACE) * (MESH_SCALE);
	//	For each plane in front-to-back or back-to-front order
	const int maxRays = MAX_RAY_STEPS * (lengthTotalModel / MESH_CUBE_DIAGONAL_LEN); // dependent on the ratio of maximum allowed rays and the current direction

	std::vector<PTVEC3> sortedProxyPlanes(maxRays);
	const int MAX_THREADS = 1;
	int samplesThread = maxRays / MAX_THREADS;
	std::vector<std::thread> threads(MAX_THREADS);

	ThreadParameters params(0, maxPos, dirSample, cubeMesh, transformedEdges, 0, 0, sortedProxyPlanes);

	//last thread gets the extra left overs
	params.threadId = MAX_THREADS - 1;
	params.first = samplesThread * (MAX_THREADS - 1) + 1;
	params.last = int(maxRays);
	//won't compile on android without std::ref
	threads[MAX_THREADS-1] = std::thread(&Renderer::calculateProxyPlanes, this, std::ref(params));

	for (int t = 0; t < MAX_THREADS-1; t++)
	{
		params.threadId = t;
		params.first = samplesThread * t + 1;
		params.last = samplesThread * (t + 1) + 1;
		params.sortedProxyPlanes = sortedProxyPlanes;
		threads[t] = std::thread(&Renderer::calculateProxyPlanes, this, std::ref(params));
		//threads[t] = std::thread(&Renderer::calculateProxyPlanes, this, maxPos, dirSample, cubeMesh, transformedEdges, sortedProxyPlanes);
	}

	for (int t = 0; t < MAX_THREADS; t++)
	{
		threads[t].join();
	}

	for (int i = 1; i < sortedProxyPlanes.size(); i++)
	{
		GLuint proxyVBO;
		glGenBuffers(1, &proxyVBO);
		glBindBuffer(GL_ARRAY_BUFFER, proxyVBO);
		glBufferData(GL_ARRAY_BUFFER, sortedProxyPlanes[i].size() * sizeof(glm::vec3), &sortedProxyPlanes[i][0], GL_STATIC_DRAW);
		GLuint position = glGetAttribLocation(shader->m_Id,"position");
		glEnableVertexAttribArray(position);
		glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glDrawArrays(GL_TRIANGLE_FAN,0, sortedProxyPlanes[i].size());
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glDeleteBuffers(1, &proxyVBO);
	}

	//delete sortedProxyPlanes;

}
#ifdef CUDA_ENABLED
void Renderer::renderRaycastVRCUDA(const Shader *shader, const Mesh &planeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize)
{


	glm::mat4 modelMatrix = planeMesh.transform.getMatrix();
	const glm::mat4 &viewMatrix = m_camera.GetViewMatrix();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat3x4 invViewMatrix = glm::mat3x4(glm::transpose(glm::inverse( modelViewMatrix)));


	//copyInvViewMatrix(glm::value_ptr(invViewMatrix), sizeof(float4)*3);


	// map PBO to get CUDA device pointer
	uint *d_output;
	// map PBO to get CUDA device pointer
	checkCudaErrorsLog(cudaGraphicsMapResources(1, &cuda_pbo_resource, 0));
	size_t num_bytes;
	checkCudaErrorsLog(cudaGraphicsResourceGetMappedPointer((void **)&d_output, &num_bytes, cuda_pbo_resource));
	//printf("CUDA mapped PBO: May access %ld bytes\n", num_bytes);

	// clear image
	checkCudaErrorsLog(cudaMemset(d_output, 0, m_screenWidth*m_screenHeight*4));

	// call CUDA kernel, writing results to PBO
	render_kernel(m_gridSize, m_blockSize, d_output, m_screenWidth, m_screenHeight, glm::value_ptr(invViewMatrix), m_aspectRatio, maxRaySteps, rayStepSize);

	getLastCudaErrorLog("kernel failed");

	checkCudaErrorsLog(cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0));

	// display results
	//glClear(GL_COLOR_BUFFER_BIT);

	// draw image from PBO
	//glDisable(GL_DEPTH_TEST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// draw using texture

	// copy from pbo to texture
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_cudaPBO);
	//glBindTexture(GL_TEXTURE_2D, m_cudaTex);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screenWidth, m_screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_cudaPBO);
	glBindTexture(GL_TEXTURE_2D, m_cudaTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screenWidth, m_screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	GLuint shaderId = shader->getId();
	glUseProgram(shaderId);
	writeUniform2DTex(shaderId, "tex", 0, m_cudaTex);
	writeUniform(shaderId, "transformMat", glm::mat4());
	planeMesh.render();

	glBindTexture(GL_TEXTURE_2D, 0);


}
#endif
void Renderer::calculateProxyPlanes(ThreadParameters &params)
{
	for (int currSample = params.first; currSample < params.last; ++currSample)
	{
		//Test for intersections with the edges of the bounding box. 
		//Add each intersection point to a temporary vertex list.
		//Up to six intersections are generated, so the maximum size of the list is fixed.
		glm::vec3 centerPt;
		PTVEC3 proxyPlane;
		getClosestPtsOnEdges(params.maxPos, params.dirSample, currSample, params.cubeMesh, params.transformedEdges, proxyPlane, centerPt);
		//	//Compute the center of the proxy polygon by averaging the intersection points.
		//Sort the polygon vertices clockwise or counterclockwise by projecting them onto the x-y plane
		//and computing their angle around the center, with the first vertex or the x axis as the reference.
		PTVEC3 sortedProxyPlane;
		sortPolygonClockwise(proxyPlane, centerPt, params.sortedProxyPlanes[currSample]);

	}
}

void Renderer::sortPolygonClockwise(const PTVEC3 &proxyPlane, glm::vec3 centerPt, PTVEC3 &sortedProxyPlane) const
{
	const glm::vec3 referencePt = proxyPlane[0];
	sortedProxyPlane.push_back(centerPt);
	sortedProxyPlane.push_back(referencePt);
	for (int i = 1; i < proxyPlane.size(); i++)
	{
		const glm::vec3 currPt = proxyPlane[i];
		//			sortedProxyPlane.push_back(currPt);
		int insertIndex = 1;
		while(insertIndex < sortedProxyPlane.size() && 
			!HelperFunctions::isLess(currPt,sortedProxyPlane.at(insertIndex), centerPt))
		{
			insertIndex++;
		}
		sortedProxyPlane.insert(sortedProxyPlane.begin() + insertIndex , currPt);
	}

	//pushback the first point on the edge of the constructed polygon to close out the triangle fan
	sortedProxyPlane.push_back(sortedProxyPlane[1]);
}

void Renderer::getClosestPtsOnEdges(const glm::vec3 &maxPos, const glm::vec3 &dirSample, int currSample, const Mesh &cubeMesh, const std::vector<Edge> &transformedEdges, PTVEC3 &proxyPlane, glm::vec3 &centerPt) const
{
	glm::vec3 currPt = glm::vec3(glm::vec4(maxPos + dirSample * (float)currSample,1.0f));
	int slotsFilled = 0;
	glm::vec3 accumulatedPt;
	//if inbetween the z coordinate of both points of the edge then the current pt intersects the edge
	for (int i = 0; i < cubeMesh.m_Edges.size() && slotsFilled < MAX_NUM_PTS_PROXY_PLANE; i++)
	{
		const glm::vec3 &p1 = transformedEdges[i].p1;
		const glm::vec3 &p2 = transformedEdges[i].p2;
		if((currPt.z < p1.z && currPt.z > p2.z) || (currPt.z < p2.z && currPt.z > p1.z))
		{
			slotsFilled++;
			glm::vec3 ptOnLine = glm::closestPointOnLine(currPt, p1, p2);
			ptOnLine.z = currPt.z;
			accumulatedPt += ptOnLine;
			proxyPlane.push_back(ptOnLine);
		}
	}
	centerPt = accumulatedPt / (float)slotsFilled;

}

void Renderer::drawCrosshair(const glm::vec4 &color) const
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	drawObject(glm::mat4(), m_crosshairPts, GL_LINES, color);
}


void Renderer::incrementRenderType()
{
	m_currRenderType++;
	if(m_currRenderType > m_availableRenderTypes.size())
		m_currRenderType = 0;
}

void Renderer::incrementTransferFnType()
{
	m_currTransferFnType++;
	if(m_currTransferFnType > m_availableTransferFnType.size())
		m_currTransferFnType = 0;
}

void Renderer::drawObject(const glm::mat4 &transformMatrix, const PTVEC3 &points, GLenum mode, const glm::vec4 &color) const
{
	GLint currShader;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currShader);
	GLuint shaderId = m_debugShader->m_Id;
	glUseProgram(shaderId);

	GLuint uniformLoc = glGetUniformLocation (shaderId, "MVP");
	glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(transformMatrix));

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), &points[0], GL_STATIC_DRAW);
	GLuint position = glGetAttribLocation(shaderId,"position");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	if(color.w != 0.0f)
		writeUniform(shaderId, "color", color);
	
	glDrawArrays(mode,0, points.size());
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDeleteBuffers(1, &vbo);

	if(color.w != 0.0f)
		writeUniform(shaderId, "color", glm::vec4(0,0,0,0));

	glUseProgram(currShader);
}


Renderer::ThreadParameters::ThreadParameters(int threadId, glm::vec3 &maxPos, glm::vec3 &dirSample, const Mesh &cubeMesh, std::vector<Edge> &transformedEdges, int first, int last, std::vector<PTVEC3> &sortedProxyPlanes)
	:threadId(threadId), maxPos(maxPos), dirSample(dirSample), cubeMesh(cubeMesh), transformedEdges(transformedEdges), first(first), last(last), sortedProxyPlanes(sortedProxyPlanes)
{

}

