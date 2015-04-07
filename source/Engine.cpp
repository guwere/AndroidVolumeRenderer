#include "Engine.h"
#include "unistd.h"

Engine& Engine::get()
{
	static Engine singleton;
	return singleton;
}

int Engine::initDisplay()
{
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	int major, minor;
	eglInitialize(display, &major, &minor);
	LOGI("EGL version %d.%d\n", major, minor);

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(mApp->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, mApp->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	this->display = display;
	this->context = context;
	this->surface = surface;
	this->width = w;
	this->height = h;
	this->state.angle = 0;

	// Initialize GL state.
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	// glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	return 0;
}

void Engine::drawFrame()
{
	if (display == NULL) {
		// No display.
		return;
	}

	// Just fill the screen with a color.
	glClearColor(((float)state.x)/width, state.angle,
		((float)state.y)/height, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(display, surface);
}

void Engine::termDisplay()
{
	if (display != EGL_NO_DISPLAY) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (context != EGL_NO_CONTEXT) {
			eglDestroyContext(display, context);
		}
		if (surface != EGL_NO_SURFACE) {
			eglDestroySurface(display, surface);
		}
		eglTerminate(display);
	}
	animating = 0;
	display = EGL_NO_DISPLAY;
	context = EGL_NO_CONTEXT;
	surface = EGL_NO_SURFACE;
}

void Engine::init(android_app *state)
{
	//set all member variables to 0
	memset(this, 0, sizeof(Engine));

	state->userData = this;
	mApp = state;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	setAssetManager();
	printInfoPath(state);

	// Prepare to monitor accelerometer
	sensorManager = ASensorManager_getInstance();
	accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager,state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		this->state = *(struct saved_state*)state->savedState;
	}
}

void Engine::printInfoPath(struct android_app* state)
{
	LOGI("Printing relevant package paths.");
	// Get usable JNI context
	JNIEnv* env = state->activity->env;
	JavaVM* vm = state->activity->vm;
	vm->AttachCurrentThread(&env, NULL);

	jclass activityClass = env->GetObjectClass(state->activity->clazz);

	jclass fileClass = env->FindClass("java/io/File");
	jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");

	{

		jmethodID method = env->GetMethodID(activityClass, "getFilesDir", "()Ljava/io/File;");
		jobject file = env->CallObjectMethod(state->activity->clazz, method);
		jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getFilesDir: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}

	{

		jmethodID method = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
		jobject file = env->CallObjectMethod(state->activity->clazz, method);
		jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getCacheDir: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}



	{

		jmethodID method = env->GetMethodID(activityClass, "getPackageResourcePath", "()Ljava/lang/String;");
		jstring jpath = (jstring)env->CallObjectMethod(state->activity->clazz, method);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getPackageResourcePath: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}

	{

		jmethodID method = env->GetMethodID(activityClass, "getPackageCodePath", "()Ljava/lang/String;");
		jstring jpath = (jstring)env->CallObjectMethod(state->activity->clazz, method);
		const char* dir = env->GetStringUTFChars(jpath, NULL);
		LOGI("getPackageCodePath: %s", dir);
		env->ReleaseStringUTFChars(jpath, dir);
	}
}

void Engine::setAssetManager()
{
	JNIEnv* env = mApp->activity->env;
	JavaVM* vm = mApp->activity->vm;
	vm->AttachCurrentThread(&env, NULL);

	// Get a handle on our calling NativeActivity instance
	jclass activityClass = env->GetObjectClass(mApp->activity->clazz);

	// Get path to cache dir (/data/data/org.wikibooks.OpenGL/cache)
	jmethodID getCacheDir = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
	jobject file = env->CallObjectMethod(mApp->activity->clazz, getCacheDir);
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
	AAssetManager* mgr = mApp->activity->assetManager;
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

void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
	Engine* engine = (Engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		engine->mApp->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->mApp->savedState) = engine->state;
		engine->mApp->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->mApp->window != NULL) {
			engine->initDisplay();
			engine->drawFrame();
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine->termDisplay();
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L/60)*1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// Also stop animating.
		engine->animating = 0;
		engine->drawFrame();
		break;
	}
}

int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	Engine* engine = (Engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->animating = 1;
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}
