#include "GLFWRenderer.h"

GLfloat GLFWRenderer::lastX = SCREEN_WIDTH * 0.5f;
GLfloat GLFWRenderer::lastY = SCREEN_HEIGHT * 0.5f;

bool GLFWRenderer::keys[1024] = {0};
bool GLFWRenderer::firstMouse = true;



GLFWRenderer::GLFWRenderer()
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

	Renderer::init(SCREEN_WIDTH, SCREEN_HEIGHT);

}

void GLFWRenderer::mainLoop()
{
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	glClear(m_clearMask);

	
	//sdkStartTimer(&(m_timer.m_timer));
	while (!glfwWindowShouldClose(window))
	{
		// Check and call events
		glfwPollEvents();
		sdkStartTimer(&m_timer.m_timer);
		

		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		glClear(m_clearMask);

		updateCallback(m_currRenderType, m_currTransferFnType, m_currVolume);
		
		glfwSwapBuffers(window);
		sdkStopTimer(&m_timer.m_timer);
		handleInput();
		std::ostringstream stringStream;
//		LOGI("FPS: %f,\n ", m_timer.computeFPS());
#ifdef CUDA_ENABLED		
		std::string currMode = (m_currRenderType == RAYTRACE_SHADER ? "RAYTRACE_SHADER":
			m_currRenderType == TEXTURE_BASED ? "TEXTURE_BASED":
			m_currRenderType == TEXTURE_BASED_MT ? "TEXTURE_BASED_MT":
			m_currRenderType == RAYTRACE_CUDA ? "RAYTRACE_CUDA": ""
			);
		m_currFPS = m_timer.computeFPS();
		stringStream << "FPS: " <<  m_currFPS << "--" << currMode;
		std::string copyOfStr = stringStream.str();
		glfwSetWindowTitle(window, copyOfStr.c_str());
#endif
	}

	glfwTerminate();
	return;

}

void GLFWRenderer::handleInput()
{
	float deltaTime = sdkGetAverageTimerValue(&m_timer.m_timer);
	//deltaTime *= 0.002f;
	//LOGI("FPS: %f,\n ", m_timer.computeFPS());

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
	if(keys[GLFW_KEY_R])
		m_constructIntersectRay = true;

	if(keys[GLFW_KEY_1])
	{
		m_currRenderType =RenderType::RAYTRACE_SHADER;
	}
	if(keys[GLFW_KEY_2])
	{
		m_currRenderType =RenderType::TEXTURE_BASED;
	}
	if(keys[GLFW_KEY_3])
	{
		m_currRenderType =RenderType::TEXTURE_BASED_MT;
	}
#ifdef CUDA_ENABLED
	if(keys[GLFW_KEY_4])
	{
		m_currRenderType =RenderType::RAYTRACE_CUDA;
	}
#endif
	if(keys[GLFW_KEY_5])
	{
		m_currVolume = 0;
	}
	if(keys[GLFW_KEY_6])
	{
		m_currVolume = 1;
	}
	if(keys[GLFW_KEY_7])
	{
		m_currTransferFnType =TransferFnType::LOW;
	}
	if(keys[GLFW_KEY_8])
	{
		m_currTransferFnType =TransferFnType::MEDIUM;
	}
	if(keys[GLFW_KEY_9])
	{
		m_currTransferFnType =TransferFnType::HIGH;
	}


}