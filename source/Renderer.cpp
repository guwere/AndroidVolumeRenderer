#include "Renderer.h"

Renderer::Renderer()
	:m_clearColor(glm::vec4(0,0,0, 1.0f)), m_clearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
{

}

Renderer::Renderer(const glm::vec4 &clearColor, const GLbitfield &clearMask)
	:m_clearColor(clearColor), m_clearMask(clearMask)
{

}

void Renderer::preRenderUpdate()
{
	glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	glClear(m_clearMask);
}

void Renderer::postRenderUpdate(GLFWwindow* window)
{
	glfwSwapBuffers(window);
}

void Renderer::renderBasic(const Shader *shader, const Mesh &mesh, const glm::mat4 &MVP) const
{
	glUseProgram(shader->getId());
	glUniformMatrix4fv(glGetUniformLocation(shader->getId(),"MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	mesh.render();
}

void Renderer::renderRaycastVR(const Shader *shader, const Mesh &cubeMesh, const glm::mat4 MVP, const Volume &volume, const glm::vec3 &eyePos, float maxRaySteps, float rayStepSize, float gradientStepSize, const glm::vec3 &lightPos, const TransferFunction &transferFn) const
{
	GLuint shaderId = shader->getId();
	glUseProgram(shader->m_Id);
	GLuint uniformLoc;
	uniformLoc = glGetUniformLocation (shaderId, "MVP");
	glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, glm::value_ptr(MVP));

	glActiveTexture (GL_TEXTURE0);
	uniformLoc = glGetUniformLocation(shaderId,"volume");
	glUniform1i(uniformLoc,0);
	glBindTexture (GL_TEXTURE_3D, volume.getTextureId());

	uniformLoc = glGetUniformLocation(shaderId,"camPos");
	glUniform3fv(uniformLoc, 1, glm::value_ptr(eyePos));

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

	cubeMesh.render();
}
