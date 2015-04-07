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

    struct android_app* mApp;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;

public:
	/**
	 * Initialize an EGL context for the current display.
	 */
	int initDisplay();

	/**
	 * Just the current frame in the display.
	 */
	void drawFrame();

	/**
	 * Tear down the EGL context currently associated with the display.
	 */
	void termDisplay();
	void init(android_app *state);
	void printInfoPath(android_app* state);
	void setAssetManager();
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