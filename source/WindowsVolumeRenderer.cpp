// WindowsVolumeRenderer.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Common.h"
#include "InitHeader.h"
#include "GLFWRenderer.h"

GLFWRenderer glfwRenderer;
int main(int argc, char* argv[])
{
	renderer = &glfwRenderer;
	initAppParams();
	glfwRenderer.setUpdateCallback(updateCallback);
	glfwRenderer.mainLoop();


	return 0;
}