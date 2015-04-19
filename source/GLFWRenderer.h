#ifndef GLFWRENDERER_H
#define GLFWRENDERER_H
#pragma once

#include "Common.h"
#include "Renderer.h"
#include "Timer.h"

class GLFWRenderer
	: public Renderer
{
public:
	GLFWwindow* window;
	GLFWRenderer();
	virtual void mainLoop();
	virtual void handleInput();
	static GLfloat lastX, lastY;
	static bool keys[1024];
	static bool firstMouse;

protected:

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		if(key == GLFW_KEY_F && action == GLFW_PRESS)
			LOGI("Framerate: %f\n", Timer::get().getFrameRate());

		if(key < 0 || key > 1024)
			return;
		if(action == GLFW_PRESS)
			keys[key] = true;
		else if(action == GLFW_RELEASE)
			keys[key] = false;	
	}


	static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if(firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		GLfloat xoffset = xpos - lastX;
		GLfloat yoffset = lastY - ypos;  

		lastX = xpos;
		lastY = ypos;

		m_camera.ProcessMouseMovement(xoffset, yoffset);
	}	


	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		m_camera.ProcessMouseScroll(yoffset);
	}


};




#endif


