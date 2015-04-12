#ifndef ENGINE_H
#define ENGINE_H
#pragma once

#include "Common.h"
/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct Engine {
public:
	static Engine& get();

    struct android_app* m_App;

    ASensorManager* m_sensorManager;
    const ASensor* m_accelerometerSensor;
    ASensorEventQueue* m_sensorEventQueue;

    int m_animating;
    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;
    int32_t m_width;
    int32_t m_height;
    struct saved_state m_state;
	void (*applicationInitCallback)(void);

public:
	void setAppState(android_app *state);
	void setApplicationInitCallback(void (*applicationInitCallback)(void));
	int  initEGL();
	/**
	 * Just the current frame in the display.
	 */
	void drawFrame();

	/**
	 * Tear down the EGL context currently associated with the display.
	 */
	void termDisplay();
	void printInfoPath();
	void printGLContextInfo();
	void setAssetManager();
private:
	Engine();
};




/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event);

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd);


#endif