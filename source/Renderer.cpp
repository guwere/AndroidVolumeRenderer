#include "Renderer.h"

Camera Renderer::m_camera;


Renderer::Renderer(float screenWidth, float screenHeight)
	:m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{
	m_clearColor = CLEAR_COLOR;
	m_clearMask = CLEAR_MASK;
	//m_camera.setPosition(INITIAL_CAMERA_POS);
	m_camera.projection = glm::perspective(CAMERA_FOV, screenWidth / screenHeight, NEAR_CLIP_PLANE, FAR_CLIP_PLANE);

}


Renderer::Renderer()
{
	m_clearColor = CLEAR_COLOR;
	m_clearMask = CLEAR_MASK;
	//glm::vec3 INITIAL_CAMERA_POS = glm::vec3(0.0f, 1.0f, 10.0f);
	//m_camera.setPosition(INITIAL_CAMERA_POS);
	int w = 0;
}

void Renderer::setUpdateCallback(void(*updateCallback)(void))
{
	this->updateCallback = updateCallback;
}

void Renderer::renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP) const
{
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
	static float rot = 0.0f;
	rot += 0.002f;
	glUniformMatrix4fv(glGetUniformLocation(shader->getId(),"MVP"), 1, GL_FALSE, glm::value_ptr(glm::rotate(MVP, rot, glm::vec3(1,1,0))));
	mesh.render();
}

void Renderer::renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const Volume &volume, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPosWorld, const TransferFunction &transferFn) const
{

	const glm::mat4 MV = m_camera.GetViewMatrix() * cubeMesh.transform.getMatrix();
	const glm::mat4 MVP = m_camera.GetProjectionMatrix() * MV;
	const glm::mat3 inverseM = glm::inverse(glm::mat3(cubeMesh.transform.getMatrix()));
	const glm::vec3 lightPos =  inverseM * lightPosWorld;

	const glm::vec3 camPos =  inverseM * m_camera.getPosition();

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
	glUniform3f(uniformLoc, camPos.x, camPos.y, camPos.z);

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
