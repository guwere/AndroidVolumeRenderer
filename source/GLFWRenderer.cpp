#include "GLFWRenderer.h"
#include "Timer.h"

GLfloat GLFWRenderer::lastX = SCREEN_WIDTH * 0.5f;
GLfloat GLFWRenderer::lastY = SCREEN_HEIGHT * 0.5f;

bool GLFWRenderer::keys[1024] = {0};
bool GLFWRenderer::firstMouse = true;



GLFWRenderer::GLFWRenderer()
	:Renderer(SCREEN_WIDTH, SCREEN_HEIGHT)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	if (!glfwInit())
		LOGI("Could not initialize glfw\n");
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, APP_NAME.c_str(), NULL, NULL);
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


}

void GLFWRenderer::mainLoop(void(*updateCallback)(void))
{
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	glClear(m_clearMask);


	while (!glfwWindowShouldClose(window))
	{
		// Check and call events
		glfwPollEvents();
		Timer::get().updateInterval();
		handleInput();

		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		glClear(m_clearMask);

		updateCallback();
		
		glfwSwapBuffers(window);


	}

	glfwTerminate();
	return;

}



void GLFWRenderer::handleInput()
{
	float deltaTime = Timer::get().getLastInterval();
	//camera
	if (keys[GLFW_KEY_ESCAPE])
		glfwSetWindowShouldClose(window, GL_TRUE);
	if(keys[GLFW_KEY_W])
		m_camera.ProcessKeyboard(FORWARD, deltaTime);
	if(keys[GLFW_KEY_S])
		m_camera.ProcessKeyboard(BACKWARD, deltaTime);
	if(keys[GLFW_KEY_A])
		m_camera.ProcessKeyboard(LEFT, deltaTime);
	if(keys[GLFW_KEY_D])
		m_camera.ProcessKeyboard(RIGHT, deltaTime);

}