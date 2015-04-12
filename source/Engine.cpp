#include "Engine.h"
#include "unistd.h"

Engine& Engine::get()
{
	static Engine singleton;
	return singleton;
}
Engine::Engine()
{
	//set all member variables to 0
	memset(this, 0, sizeof(Engine));
}


void Engine::setAppState(android_app *state)
{
	m_App = state;
	m_App->userData = this;
	m_App->onAppCmd = engine_handle_cmd;
	m_App->onInputEvent = engine_handle_input;
	setAssetManager();
	printInfoPath();
}

void Engine::setApplicationInitCallback(void (*applicationInitCallback)(void))
{
	this->applicationInitCallback = applicationInitCallback;
}


int Engine::initEGL()
{
	// Prepare to monitor accelerometer
	m_sensorManager = ASensorManager_getInstance();
	m_accelerometerSensor = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager,m_App->looper, LOOPER_ID_USER, NULL, NULL);

	if (m_App->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		this->m_state = *(struct saved_state*)m_App->savedState;
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

	const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, //Request opengl ES3.0
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
	this->m_width = w;
	this->m_height = h;
	this->m_state.angle = 0;

	// Initialize GL state.
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	//glEnable(GL_CULL_FACE);
	// glShadeModel(GL_SMOOTH);
	//glDisable(GL_DEPTH_TEST);

	return 0;
}

void Engine::drawFrame()
{
	if (m_display == NULL) {
		// No display.
		return;
	}

	// Just fill the screen with a color.
	glClearColor(((float)m_state.x)/m_width, m_state.angle,
		((float)m_state.y)/m_height, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(m_display, m_surface);
}

void Engine::termDisplay()
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



void Engine::printInfoPath()
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

void Engine::setAssetManager()
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


void Engine::printGLContextInfo()
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
	Engine* engine = (Engine*)app->userData;
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
			engine->applicationInitCallback();
			engine->drawFrame();
		}
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
		engine->drawFrame();
		break;
	}
}

int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	Engine* engine = (Engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->m_animating = 1;
		engine->m_state.x = AMotionEvent_getX(event, 0);
		engine->m_state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}
