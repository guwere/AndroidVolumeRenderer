#include "Renderer.h"
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/intersect.hpp>
#include "HelperFunctions.h"
#include "ShaderManager.h"
Camera Renderer::m_camera;


Renderer::Renderer(float screenWidth, float screenHeight)
	:m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{
	m_clearColor = CLEAR_COLOR;
	m_clearMask = CLEAR_MASK;
	constructIntersectRay = false;
	//m_camera.setPosition(INITIAL_CAMERA_POS);
	m_camera.projection = glm::perspective(CAMERA_FOV, screenWidth / screenHeight, NEAR_CLIP_PLANE, FAR_CLIP_PLANE);
}


Renderer::Renderer()
{
	m_clearColor = CLEAR_COLOR;
	m_clearMask = CLEAR_MASK;
	constructIntersectRay = false;
	//glm::vec3 INITIAL_CAMERA_POS = glm::vec3(0.0f, 1.0f, 10.0f);
	//m_camera.setPosition(INITIAL_CAMERA_POS);
}

void Renderer::getMinMaxPoints(const glm::mat4 &modelViewMatrix, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const
{
	glm::mat4 inverseP = glm::inverse(m_camera.GetProjectionMatrix());
	glm::vec4 vertexView = modelViewMatrix * glm::vec4(cubeMesh.m_Vertices[0].m_Position, 1.0f);
	//float distanceToCamera = abs(vertexView.z);
	glm::vec4 origin = inverseP * glm::vec4(0,0,0,1);
	float distanceToCamera = glm::length2(vertexView - origin);
	float currMin = distanceToCamera;
	float currMax = distanceToCamera;
	glm::vec4 currMinPt = vertexView;
	glm::vec4 currMaxPt = vertexView;
	int currMinIndex = 0;
	int currMaxIndex = 0;
	std::vector<glm::vec4> temp;
	for (unsigned int i = 1; i < cubeMesh.m_Vertices.size(); ++i)
	{
		vertexView = modelViewMatrix * glm::vec4(cubeMesh.m_Vertices[i].m_Position, 1.0f);
		temp.push_back(vertexView);
		//distanceToCamera = abs(vertexView.z);
		distanceToCamera = glm::length2(vertexView - origin);
		if(distanceToCamera < currMin)
		{
			currMin = distanceToCamera;
			currMinPt = vertexView;
			currMinIndex = i;
		}
		else if(distanceToCamera > currMax)
		{
			currMax = distanceToCamera;
			currMaxPt = vertexView;
			currMaxIndex = i;
		}
	}
	minPos = cubeMesh.m_Vertices[currMinIndex].m_Position;
	maxPos = cubeMesh.m_Vertices[currMaxIndex].m_Position;



}



void Renderer::getMinMaxPoints2(const glm::mat4 &modelViewMatrix, const Mesh &cubeMesh, glm::vec3 &minPos, glm::vec3 &maxPos) const
{
	int intersections = 0;
	std::vector<glm::vec3> intersectioPts;
	std::vector<glm::vec3> intersectionVertices;
	float currMin = FLT_MAX;
	float currMax = FLT_MIN;
	glm::vec3 currMinPt;
	glm::vec3 currMaxPt;
	glm::vec3 front = m_camera.Front;
	glm::vec3 position = m_camera.getPosition();
	
	glm::mat4 inverseM = glm::inverse(cubeMesh.transform.getMatrix());
	glm::mat4 inverseP = glm::inverse(m_camera.GetProjectionMatrix());
	glm::mat4 viewMatrix = m_camera.GetViewMatrix();
	glm::mat4 modelMatrix = cubeMesh.transform.getMatrix();
	glm::mat4 inverseMV = glm::inverse(modelViewMatrix);
	glm::mat4 inverseMVP = m_camera.GetProjectionMatrix() * viewMatrix * modelMatrix;
	glm::vec3 origin = glm::vec3(inverseM * glm::vec4(position,1));
	//glm::vec3 origin = glm::vec3(inverseP * glm::vec4(glm::vec3(0), 1.0f));
	glm::vec3 direction  = glm::vec3(inverseM * glm::vec4(front,1));
	//glm::vec3 direction  = glm::vec3(0,0,-1);
	for (unsigned int i = 0; i < cubeMesh.m_Indices.size(); i +=3)
	{

		glm::vec3 v1 = glm::vec3(glm::vec4(cubeMesh.m_Vertices[cubeMesh.m_Indices[i]].m_Position, 1.0f));
		glm::vec3 v2 = glm::vec3(glm::vec4(cubeMesh.m_Vertices[cubeMesh.m_Indices[i+1]].m_Position, 1.0f));
		glm::vec3 v3 = glm::vec3(glm::vec4(cubeMesh.m_Vertices[cubeMesh.m_Indices[i+2]].m_Position, 1.0f));

		glm::vec3 uvtCoord;
		//if(glm::intersectRayTriangle(origin, direction , vertexView1, vertexView2, vertexView3, intersectPos))
		if(HelperFunctions::intersectRayTriangle(v1, v2, v3, true, origin, direction , uvtCoord))
		{
			glm::vec3 d1 = (v2 - v1) * uvtCoord.x;
			glm::vec3 d2 = (v3 - v1) * uvtCoord.y;
			glm::vec3 intersecPt = v1 + (d1 + d2); 
			intersectioPts.push_back(intersecPt);
			intersectionVertices.push_back(glm::vec3(cubeMesh.m_Indices[i],cubeMesh.m_Indices[i+1],cubeMesh.m_Indices[i+2]));
		}
	}
	if(intersectioPts.size() < 2)
		return;
	else if(intersectioPts.size() == 1)
	{
		return;
	}
	float distance1 = glm::length2(intersectioPts.at(0) - origin);
	float distance2 = glm::length2(intersectioPts.at(1) - origin);
	if(distance1 < distance2)
	{
		minPos = glm::vec3(glm::vec4(intersectioPts.at(0), 1.0f));
		maxPos = glm::vec3(glm::vec4(intersectioPts.at(1), 1.0f));
	}
	else
	{
		minPos = glm::vec3(glm::vec4(intersectioPts.at(1), 1.0f));
		maxPos = glm::vec3(glm::vec4(intersectioPts.at(0), 1.0f));
	}
	//minPos = glm::vec3(currMinPt);
	//maxPos = glm::vec3(currMaxPt);
	//minPos = glm::vec3(inverseMV * glm::vec4(currMinPt, 1.0f));
	//maxPos = glm::vec3(inverseMV * glm::vec4(currMaxPt, 1.0f));
	int x = 0;
}

void Renderer::loadDebugShader()
{
	//File vs("", std::string(DEBUG_VERTEX_SHADER_NAME));
	//File fs("", std::string(DEBUG_FRAGMENT_SHADER_NAME));
	m_debugShader = ShaderManager::get().getShader(File("", std::string(DEBUG_VERTEX_SHADER_NAME)),File("", std::string(DEBUG_FRAGMENT_SHADER_NAME)));
}

void Renderer::setUpdateCallback(void(*updateCallback)(void))
{
	this->updateCallback = updateCallback;
}

void Renderer::renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP, bool renderWireframe) const
{
	glEnable(GL_DEPTH_TEST);
	glUseProgram(shader->getId());
	//float mymat[16] = {
	//	0.597530365,
	//	0.000000000,
	//	0.000000000,
	//	0.000000000,
	//	0.000000000,
	//	0.896295488,
	//	0.000000000,
	//	0.000000000,
	//	0.000000000,
	//	0.000000000,
	//	-0.500000954,
	//	-0.500000954,
	//	0.000000000,
	//	-1.79259098,
	//	4.99800968,
	//	5.00000000
	//};
	//static float rot = 0.0f;
	//rot += 0.002f;
	if(renderWireframe)
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glUniformMatrix4fv(glGetUniformLocation(shader->getId(),"MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	mesh.render();
	if(renderWireframe)
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);

	//glm::vec3 pt1(0);
	//glm::vec3 pt2(-1);
	////pt1 = glm::vec3(m_camera.GetViewMatrix() * glm::vec4(pt1,1.0f));
	//pt2 = glm::vec3(MVP * glm::vec4(pt2,1.0f));
	//std::vector<glm::vec3> linePts;
	//linePts.push_back(pt1);
	//linePts.push_back(pt2);
	//drawLine(glm::mat4(), linePts);
	//drawLine(glm::mat4(), linePts);

	glDisable(GL_DEPTH_TEST);

}

void Renderer::renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const
{
	glEnable(GL_DEPTH_TEST);
	const glm::mat4 MV = m_camera.GetViewMatrix() * cubeMesh.transform.getMatrix();
	const glm::mat4 MVP = m_camera.GetProjectionMatrix() * MV;
	const glm::mat3 inverseM = glm::inverse(glm::mat3(cubeMesh.transform.getMatrix()));
	const glm::vec3 lightPos =  inverseM * lightPosWorld;

	const glm::vec3 camPosModel =  inverseM * m_camera.getPosition();

	GLuint shaderId = shader->getId();
	glUseProgram(shader->m_Id);
	GLuint uniformLoc;
	uniformLoc = glGetUniformLocation (shaderId, "MVP");
	glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(MVP));

	//uniformLoc = glGetUniformLocation (shaderId, "modelMatrix");
	//glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(cubeMesh.transform.getMatrix()));


	glActiveTexture (GL_TEXTURE0);
	uniformLoc = glGetUniformLocation(shaderId,"volume");
	glUniform1i(uniformLoc,0);
	glBindTexture (GL_TEXTURE_3D, volume.getTextureId());

	uniformLoc = glGetUniformLocation(shaderId,"camPos");
	glUniform3f(uniformLoc, camPosModel.x, camPosModel.y, camPosModel.z);

	uniformLoc = glGetUniformLocation(shaderId, "maxRaySteps");
	glUniform1i(uniformLoc, maxRaySteps);

	uniformLoc = glGetUniformLocation(shaderId, "rayStepSize");
	glUniform1f(uniformLoc, rayStepSize);

	uniformLoc = glGetUniformLocation(shaderId, "gradientStepSize");
	glUniform1f(uniformLoc, gradientStepSize);


	uniformLoc = glGetUniformLocation(shaderId,"lightPosition");
	glUniform3fv(uniformLoc, 1, glm::value_ptr(lightPos));

	glActiveTexture (GL_TEXTURE1);
	uniformLoc = glGetUniformLocation(shaderId,"transferFunc");
	glUniform1i(uniformLoc,1);
	glBindTexture (GL_TEXTURE_2D, transferFn.getTextureId());

	cubeMesh.render(GL_TRIANGLES, true);
}

void Renderer::renderTextureBasedVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float numSamples, const TransferFunction &transferFn)
{
	const glm::mat4 modelMatrix = cubeMesh.transform.getMatrix();
	const glm::mat4 viewMatrix = m_camera.GetViewMatrix();
	const glm::mat4 projMatrix = m_camera.GetProjectionMatrix();
	glm::mat4 modelViewMatrix = viewMatrix * cubeMesh.transform.getMatrix();
	glm::mat4 MVPMatrix = projMatrix * viewMatrix * modelMatrix;
	glm::mat3 inverseMVPMatrix = glm::mat3(glm::inverse(MVPMatrix));
	glm::mat4 inverseMV = glm::inverse(modelViewMatrix);

	GLuint shaderId = shader->getId();
	glUseProgram(shader->m_Id);
	//write uniform variables
	GLuint uniformLoc;
	glActiveTexture (GL_TEXTURE0);
	uniformLoc = glGetUniformLocation(shaderId,"volume");
	glUniform1i(uniformLoc,0);
	glBindTexture (GL_TEXTURE_3D, volume.getTextureId());
	glActiveTexture (GL_TEXTURE1);
	uniformLoc = glGetUniformLocation(shaderId,"transferFunc");
	glUniform1i(uniformLoc,1);
	glBindTexture (GL_TEXTURE_2D, transferFn.getTextureId());

	uniformLoc = glGetUniformLocation (shaderId, "projMatrix");
	glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(m_camera.GetProjectionMatrix()));
	uniformLoc = glGetUniformLocation (shaderId, "inverseMV");
	glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(inverseMV));

	//enable proper alpha blending for back-to-front order
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	//do the actual texture based algorithm
	glm::vec3 minPos, maxPos;
	//Find the minimum and maximum z coordinates of transformed vertices in view space
	getMinMaxPoints2(modelViewMatrix, cubeMesh, minPos, maxPos);
	if(constructIntersectRay)
	{
		rayStart = minPos;
		rayEnd = maxPos;
		constructIntersectRay = false;
	}
	//glm::vec3 minposview = glm::vec3(modelViewMatrix * glm::vec4(minPos,1.0f));
	//glm::vec3 maxposview = glm::vec3(modelViewMatrix * glm::vec4(maxPos,1.0f));

	std::vector<glm::vec3> linePts;
	linePts.push_back(rayStart);
	linePts.push_back(rayEnd);
	drawObject(MVPMatrix, linePts, GL_LINE_STRIP, glm::vec4(0,1,1,1));

	//linePts.push_back(glm::vec3(-0.2f, -0.2f, 0.2f));
	////linePts.push_back(glm::vec3(projMatrix * viewMatrix * glm::vec4(maxPos, 1.0f)));
	//linePts.push_back(glm::vec3(glm::vec4(maxPos, 1.0f)));
	//drawObject(MVPMatrix, linePts, GL_LINE_STRIP, glm::vec4(0,1,1,1));
	//linePts.clear();
	//linePts.push_back(glm::vec3(-0.3f, -0.3f, 0.3f));
	////linePts.push_back(glm::vec3(projMatrix * viewMatrix * glm::vec4(minPos, 1.0f)));
	//linePts.push_back(glm::vec3(glm::vec4(minPos, 1.0f)));
	//drawObject(MVPMatrix, linePts, GL_LINE_STRIP, glm::vec4(0,0,1,1));
	linePts.clear();
	linePts.push_back(glm::vec3(0));
	linePts.push_back(glm::vec3(MVPMatrix * glm::vec4(0,0,0, 1.0f)));
	drawObject(glm::mat4(), linePts, GL_LINE_STRIP, glm::vec4(0,0,1,1));

	//Compute the number of sampling planes used between these two values using equidistant spacing from the view origin
	std::vector<glm::vec3> samplingPlanePts;
	samplingPlanePts.resize((int)numSamples);
	glm::vec3 dir = minPos-maxPos;
	float lengthTotal = glm::length(dir);
	dir = glm::normalize(dir);
	float lengthSample = lengthTotal / numSamples;
	glm::vec3 dirSample = dir * lengthSample;
	std::vector<glm::vec3> centers;
//	For each plane in front-to-back or back-to-front order
	for (int currSample = 999; currSample < (int)999; ++currSample)
	{
		std::vector<glm::vec3> proxyPlane;
		//Test for intersections with the edges of the bounding box. 
		//Add each intersection point to a temporary vertex list.
		//Up to six intersections are generated, so the maximum size of the list is fixed.
		glm::vec3 currPt = glm::vec3(modelViewMatrix * glm::vec4(maxPos + dirSample * (float)currSample,1.0f));
		int slotsFilled = 0;
		glm::vec3 accumulatedPt;
		//if inbetween the z coordinate of both points of the edge then the current pt intersects the edge
const int MAX_NUM_PTS_PROXY_PLANE  = 6;
		for (int i = 0; i < cubeMesh.m_Edges.size() && slotsFilled < MAX_NUM_PTS_PROXY_PLANE; i++)
		{
			const glm::vec3 &p1 = glm::vec3(modelViewMatrix * glm::vec4(cubeMesh.m_Vertices[cubeMesh.m_Edges[i].p1].m_Position, 1.0f));
			const glm::vec3 &p2 = glm::vec3(modelViewMatrix * glm::vec4(cubeMesh.m_Vertices[cubeMesh.m_Edges[i].p2].m_Position, 1.0f));
			if((currPt.z < p1.z && currPt.z > p2.z) || (currPt.z < p2.z && currPt.z > p1.z))
			{
				slotsFilled++;
				glm::vec3 ptOnLine = glm::closestPointOnLine(currPt, p1, p2);
				const glm::vec3 &p1Model = glm::vec3(inverseMV * glm::vec4(p1, 1.0f));
				const glm::vec3 &p2Model = glm::vec3(inverseMV * glm::vec4(p2, 1.0f));
				const glm::vec3 &ptOnLineModel = glm::vec3(inverseMV * glm::vec4(ptOnLine, 1.0f));
				//ptOnLine.z = currPt.z;
				accumulatedPt += ptOnLine;
				proxyPlane.push_back(ptOnLine);
			}
		}
		//proxyPlane.pop_back();
		//drawLine(m_camera.GetProjectionMatrix(), proxyPlane);
		if(slotsFilled < 3)
		{
			int x = 0;
			continue;
			//throw std::runtime_error("Error generating vertices for proxy plane.");
		}
		if(slotsFilled == 3)
		{
			int x = 0;
		}
		else if(slotsFilled == 4)
		{
			int x = 0;
		}
		else if(slotsFilled == 5)
		{
			int x = 0;
		}
		else if(slotsFilled == 6)
		{
			int x = 0;
		}
		//drawObject(MVPMatrix, proxyPlane, GL_LINE_STRIP, glm::vec4(0,1.0f,0, 1.0f));

	//	//Compute the center of the proxy polygon by averaging the intersection points.
		glm::vec3 centerPt = accumulatedPt / (float)slotsFilled;
		centers.push_back(centerPt);
		//Sort the polygon vertices clockwise or counterclockwise by projecting them onto the x-y plane
		//and computing their angle around the center, with the first vertex or the x axis as the reference.
		const glm::vec3 referencePt = proxyPlane[0];
		std::vector<glm::vec3> sortedProxyPlane;
		int sizex = sortedProxyPlane.size();
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

		//Tessellate the proxy polygon into triangles and add the resulting vertices to the output vertex array. 
		//The slice polygon can be tessellated into a triangle strip or a triangle fan using the center.
		//Depending on the rendering algorithm, the vertices may need to be transformed back to object space during this step.
		GLuint proxyVBO;
		glGenBuffers(1, &proxyVBO);
		glBindBuffer(GL_ARRAY_BUFFER, proxyVBO);
		glBufferData(GL_ARRAY_BUFFER, sortedProxyPlane.size() * sizeof(glm::vec3), &sortedProxyPlane[0], GL_STATIC_DRAW);
		GLuint position = glGetAttribLocation(shader->m_Id,"position");
		glEnableVertexAttribArray(position);
		glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glDrawArrays(GL_TRIANGLE_FAN,0, sortedProxyPlane.size());
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glDeleteBuffers(1, &proxyVBO);

	}
	if(centers.size())
		drawObject(m_camera.GetProjectionMatrix(), centers, GL_LINE_STRIP, glm::vec4(1));
}

void Renderer::drawObject(const glm::mat4 &transformMatrix, const std::vector<glm::vec3> &points, GLenum mode, glm::vec4 color) const
{
	GLint currShader;
	glGetIntegerv(GL_CURRENT_PROGRAM,&currShader);
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

	uniformLoc = glGetUniformLocation (shaderId, "color");
	if(color.w != 0.0f)
	{
		glUniform4fv(uniformLoc, 1, glm::value_ptr(color));
	}
	
	glDrawArrays(mode,0, points.size());
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDeleteBuffers(1, &vbo);

	if(color.w != 0.0f)
	{
		glUniform4fv(uniformLoc, 1, glm::value_ptr(glm::vec4(0,0,0,0)));
	}

	glUseProgram(currShader);
}
