#include "AndroidRenderer.h"
#include "unistd.h"
#include "Timer.h"
#include "HelperFunctions.h"

AndroidRenderer::AndroidRenderer()
{
}


AndroidRenderer::~AndroidRenderer()
{
}

void AndroidRenderer::init(android_app *state, void (*applicationInitCallback)(void))
{
	m_clockText = NULL;
	m_App = state;
	m_applicationInitCallback = applicationInitCallback;
	m_App->userData = this;
	m_App->onAppCmd = engine_handle_cmd;
	m_App->onInputEvent = engine_handle_input;


}


int AndroidRenderer::initEGL()
{
	LOGI("Initializing EGL Context\n");

	// Prepare to monitor accelerometer
	m_sensorManager = ASensorManager_getInstance();
	m_accelerometerSensor = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager,m_App->looper, LOOPER_ID_USER, NULL, NULL);

	if (m_App->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		this->m_state = *(struct saved_state*)m_App->savedState;
	}

	setAssetManager();
	printInfoPath();

	if(EGL_BAD_DISPLAY == eglGetError())
	{
		LOGI("EGLDisplay argument does not name a valid EGL display connection");
	}


	// initialize OpenGL ES and EGL
		EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	int major, minor;
	eglInitialize(display, &major, &minor);
	LOGI("EGL version %d.%d\n", major, minor);

     /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = { EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT, 
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_NONE };

    EGLint num_configs;
    eglChooseConfig( display, attribs, &config, 1, &num_configs );
    if( !num_configs )
    {
        //Fall back to 16bit depth buffer
        const EGLint attribs[] = { EGL_RENDERABLE_TYPE,
                EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_NONE };
        eglChooseConfig( display, attribs, &config, 1, &num_configs );
    }
    if( !num_configs )
    {
        LOGI( "Unable to retrieve EGL config" );
        return false;
    }

	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);


	surface = eglCreateWindowSurface(display, config, m_App->window, NULL);
	ANativeWindow_setBuffersGeometry(m_App->window, 0, 0, format);

	const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, //Request opengl ES3.0
		EGL_NONE };
	context = eglCreateContext(display, config, NULL, context_attribs);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	this->m_display = display;
	this->m_context = context;
	this->m_surface = surface;
	this->m_screenWidth = w;
	this->m_screenHeight = h;

	Renderer::init(w,h);
	// Initialize GL state.
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	//glEnable(GL_CULL_FACE);
	// glShadeModel(GL_SMOOTH);
	//glDisable(GL_DEPTH_TEST);
	return 0;
}


void AndroidRenderer::termDisplay()
{
	if (m_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (m_context != EGL_NO_CONTEXT) {
			eglDestroyContext(m_display, m_context);
		}
		if (m_surface != EGL_NO_SURFACE) {
			eglDestroySurface(m_display, m_surface);
		}
		eglTerminate(m_display);
	}
	m_animating = 0;
	m_display = EGL_NO_DISPLAY;
	m_context = EGL_NO_CONTEXT;
	m_surface = EGL_NO_SURFACE;
}



void AndroidRenderer::printInfoPath()
{
	LOGI("Printing relevant package paths.");
	// Get usable JNI context
	JNIEnv* env = m_App->activity->env;
	JavaVM* vm = m_App->activity->vm;
	vm->AttachCurrentThread(&env, NULL);

	jclass activityClass = env->GetObjectClass(m_App->activity->clazz);

	jclass fileClass = env->FindClass("java/io/File");
	jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");

	{

		jmethodID method = env->GetMethodID(activityClass, "getFilesDir", "()Ljava/io/File;");
		jobject file = env->CallObjectMethod(m_App->activity->clazz, method);
		jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getFilesDir: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}

	{

		jmethodID method = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
		jobject file = env->CallObjectMethod(m_App->activity->clazz, method);
		jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getCacheDir: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}



	{

		jmethodID method = env->GetMethodID(activityClass, "getPackageResourcePath", "()Ljava/lang/String;");
		jstring jpath = (jstring)env->CallObjectMethod(m_App->activity->clazz, method);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getPackageResourcePath: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}

	{

		jmethodID method = env->GetMethodID(activityClass, "getPackageCodePath", "()Ljava/lang/String;");
		jstring jpath = (jstring)env->CallObjectMethod(m_App->activity->clazz, method);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getPackageCodePath: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}
}

void AndroidRenderer::setAssetManager()
{
	JNIEnv* env = m_App->activity->env;
	JavaVM* vm = m_App->activity->vm;
	vm->AttachCurrentThread(&env, NULL);

	// Get a handle on our calling NativeActivity instance
	jclass activityClass = env->GetObjectClass(m_App->activity->clazz);

	// Get path to cache dir (/data/data/org.wikibooks.OpenGL/cache)
	jmethodID getCacheDir = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
	jobject file = env->CallObjectMethod(m_App->activity->clazz, getCacheDir);
	jclass fileClass = env->FindClass("java/io/File");
	jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
	jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
	const char* app_dir = env->GetStringUTFChars(jpath, NULL);
	// chdir in the application cache directory
	LOGI("getAbsolutePath: %s", app_dir);
	LOGI("Setting current working directory to absolute path.");

	chdir(app_dir);
	
	env->ReleaseStringUTFChars(jpath, app_dir);
	// Pre-extract assets, to avoid Android-specific file opening
	AAssetManager* mgr = m_App->activity->assetManager;
	//mApp->activity->externalDataPath;
	AAssetDir* assetDir = AAssetManager_openDir(mgr, "");
	const char* filename = (const char*)NULL;
	LOGI("Printing filenames of the available assets");
	while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
		LOGI("%s\n",filename);
		AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING);
		char buf[BUFSIZ];
		int nb_read = 0;
		FILE* out = fopen(filename, "w");
		while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
			fwrite(buf, nb_read, 1, out);
		fclose(out);
		AAsset_close(asset);
	}
	AAssetDir_close(assetDir);
}


void AndroidRenderer::printGLContextInfo()
{
	
	LOGI("--OpenGL Context Information---");
	const char* str = (const char*) glGetString( GL_VERSION );
	LOGI("GL_VERSION : %s\n",str);
	str = (const char*) glGetString( GL_VENDOR );
	LOGI("GL_VENDOR : %s\n",str);
	str = (const char*) glGetString( GL_RENDERER );
	LOGI("GL_RENDERER : %s\n",str);
	str = (const char*) glGetString( GL_SHADING_LANGUAGE_VERSION );
	LOGI("GL_SHADING_LANGUAGE_VERSION : %s\n",str);


}

void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
	AndroidRenderer* engine = (AndroidRenderer*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		engine->m_App->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->m_App->savedState) = engine->m_state;
		engine->m_App->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->m_App->window != NULL) {
			engine->initEGL();
			engine->printGLContextInfo();
			engine->m_applicationInitCallback();
		}
		engine->m_animating = 1;
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine->termDisplay();
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (engine->m_accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->m_sensorEventQueue,
				engine->m_accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(engine->m_sensorEventQueue,
				engine->m_accelerometerSensor, (1000L/60)*1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (engine->m_accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->m_sensorEventQueue,
				engine->m_accelerometerSensor);
		}
		// Also stop animating.
		engine->m_animating = 0;
		//glClearColor(0.46f, 0.53f, 0.6f, 1.0f);
		glClearColor(engine->m_clearColor.r, engine->m_clearColor.g, engine->m_clearColor.b, engine->m_clearColor.a);
		glClear(engine->m_clearMask);
		engine->updateCallback(engine->m_currRenderType, engine->m_currTransferFnType);
		eglSwapBuffers(engine->m_display, engine->m_surface);

		break;
	case APP_CMD_START:

		break;
	case APP_CMD_INPUT_CHANGED:
	case APP_CMD_WINDOW_RESIZED:
	case APP_CMD_WINDOW_REDRAW_NEEDED:
	case APP_CMD_CONTENT_RECT_CHANGED:
	case APP_CMD_CONFIG_CHANGED:
	case APP_CMD_LOW_MEMORY:
	case APP_CMD_RESUME:
	case APP_CMD_PAUSE:
	case APP_CMD_STOP:
	case APP_CMD_DESTROY:
		break;
	}

}

int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	AndroidRenderer* engine = (AndroidRenderer*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		engine->m_animating = 1;
		int32_t action = AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction((const AInputEvent*)event);
		if (action == AMOTION_EVENT_ACTION_DOWN)
		{
			engine->m_state.x = AMotionEvent_getX(event, 0);
			engine->m_state.y = AMotionEvent_getY(event, 0);
			engine->m_screenPressed = true;
		}
		else if (action == AMOTION_EVENT_ACTION_UP)
		{
			engine->m_screenPressed = false;
			engine->m_screenReleased = true;
		}
		return 1;
	}
	return 0;
}


void AndroidRenderer::mainLoop()
{
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;
		float fps = 0;


		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident=ALooper_pollAll(m_animating ? 0 : -1, NULL, &events,(void**)&source)) >= 0) 
		{
			sdkStartTimer(&m_timer.m_timer);
			// Process this event.
			if (source != NULL)
			{
				source->process(m_App, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER)
			{
				if (m_accelerometerSensor != NULL)
				{
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(m_sensorEventQueue,&event, 1) > 0) {
						//LOGI("accelerometer: x=%f y=%f z=%f",event.acceleration.x, event.acceleration.y,event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (m_App->destroyRequested != 0) {
				termDisplay();
				return;
			}

		}

		if(m_screenReleased)
		{
			m_screenReleased = false;
			//GLfloat xoffset = m_state.x - m_screenWidth * 0.5;
			//GLfloat yoffset = m_state.y - m_screenHeight * 0.5;
			//m_camera.ProcessMouseMovement(xoffset * fps, -yoffset * fps);
			if(m_state.x < m_screenWidth * 0.5f && m_state.y < m_screenHeight * 0.5f)
				incrementRenderType();
			else if(m_state.x > m_screenWidth * 0.5f && m_state.y > m_screenHeight * 0.5f)
				incrementTransferFnType();

		}

		if(m_animating)
		{
			//glClearColor(0.46f, 0.53f, 0.6f, 1.0f);
			glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
			glClear(m_clearMask);
			updateCallback(m_currRenderType, m_currTransferFnType);
			eglSwapBuffers(m_display, m_surface);

		}
		sdkStopTimer(&m_timer.m_timer);
		fps = m_timer.computeFPS();
		LOGI("FPS: %f\n",fps);
		//sdkStartTimer(&m_timer.m_timer);

	}

}


void AndroidRenderer::handleInput()
{

}
