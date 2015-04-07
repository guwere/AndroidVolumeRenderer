// WindowsVolumeRenderer.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Common.h"
#include "File.h"
#include "Volume.h"
#include "Camera.h"
#include "Timer.h"
#include "Mesh.h"
#include "ShaderManager.h"
#include "TransferFunction.h"
#include "Renderer.h"
#include <GLFW/glfw3.h>

#pragma region WindowContext
GLFWwindow* window;
bool keys[1024];
GLfloat lastX = SCREEN_WIDTH * 0.5F, lastY = SCREEN_HEIGHT * 0.5f;
bool firstMouse = true;
#pragma endregion

#pragma region VolumeDeclarations 
Mesh mesh; // gio would be proud of the pragma regions !
Shader *basicShader, *raycastVRShader;
Volume volume;
TransferFunction transferFn;
Renderer renderer;
Camera camera;
#pragma endregion

#pragma region VolumeProperties
const glm::vec3 initialCameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
const int maxRaySteps = 1000;
const float rayStepSize = 0.005f;
const float gradientStepSize = 0.005f;
const glm::vec3 lightPos = glm::vec3(-2.0f, 2.0f, 2.0f);
#pragma endregion 

#pragma region FunctionPrototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void handleInput();
void setup();
#pragma endregion

int main(int argc, char* argv[])
{

	//File myFile("","CTknee.mhd");
	//Volume myVolume;
	//myVolume.parseMHD(myFile);
	//myVolume.printProperties();
	//File myFile("","helloworld.txt");
	//File myFile2("","second.txt");
	//myFile.append(myFile2);
	//myFile.print();
	//Initialize OpenGL and GLFW
	setup();


	while (!glfwWindowShouldClose(window))
	{
		// Check and call events
		glfwPollEvents();
		handleInput();
		volume.advance();
		glm::mat4 MVP = camera.GetProjectionMatrix() * camera.GetViewMatrix() * mesh.transform.getMatrix();
		glm::vec3 cameraPos = camera.getPosition();


		renderer.preRenderUpdate();
		/* Render here */
		//renderer.renderBasic(basicShader, mesh, camera.GetProjectionMatrix() * camera.GetViewMatrix() * mesh.transform.getMatrix());
		
		renderer.renderRaycastVR(raycastVRShader, mesh, MVP, volume, cameraPos, maxRaySteps, rayStepSize, gradientStepSize, lightPos, transferFn);
		renderer.postRenderUpdate(window);

	}

	glfwTerminate();
	return 0;
}

void handleInput()
{
	Timer::get().updateInterval();
	float deltaTime = Timer::get().getLastInterval();
	float time = Timer::get().getTime();
	glm::vec4 cameraPosition = (camera.GetViewMatrix())[3];
	//camera
	if (keys[GLFW_KEY_ESCAPE])
		glfwSetWindowShouldClose(window, GL_TRUE);
	if(keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if(keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if(keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if(keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);

}


void setup()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	if (!glfwInit())
		LOGI("Could not initialize glfw\n");
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,APP_NAME, NULL, NULL);
	if (!window)
	{
		LOGI("Could not create window context\n");
		glfwTerminate();
	}
	else
		LOGI("Window context created\n");
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	//Register callbacks
	glfwSetKeyCallback(window, key_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		LOGI("%s\n",glewGetErrorString(err));
	}

	camera.Position = initialCameraPos;
	camera.projection = glm::perspective(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
	//basicShader = ShaderManager::get().getShader(File("","basicVS.glsl"), File("","basicFS.glsl"));
	raycastVRShader = ShaderManager::get().getShader(File("", "raycastVRShaderVS.glsl"), File("", "raycastVRShaderFS.glsl"));
	mesh.generateCube(raycastVRShader);

	volume.parseMHD(File("","CTknee.mhd"));
	volume.generate();
	transferFn.parseVoreenXML(File("","CT-Knee_spectrum_16_balance.tfi"));
	transferFn.generate1DTexture();
	int x = 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);


	if(action == GLFW_PRESS)
		keys[key] = true;
	else if(action == GLFW_RELEASE)
		keys[key] = false;	
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}	


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
