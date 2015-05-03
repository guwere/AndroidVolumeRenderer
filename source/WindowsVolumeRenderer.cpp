// WindowsVolumeRenderer.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Common.h"
#include "InitHeader.h"
#include "GLFWRenderer.h"

int main(int argc, char* argv[])
{
	GLFWRenderer glfwRenderer;
	renderer = &glfwRenderer;
	initAppParams();
	glfwRenderer.setUpdateCallback(updateCallback);
	glfwRenderer.mainLoop();


	return 0;
}