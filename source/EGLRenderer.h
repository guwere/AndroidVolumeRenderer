#ifndef EGLRENDERER_H
#define EGLRENDERER_H
#pragma once

#include "Common.h"
#include "Renderer.h"
#include "Timer.h"
#include "Engine.h"

class EGLRenderer
	: public Renderer
{
public:
	EGLRenderer();
	virtual void mainLoop(void(*updateCallback)(void));
	virtual void handleInput();
	void setInitApplicationCallback(void (*applicationInitCallback)(void));
protected:
	void init(struct android_app* state, void (*applicationInitCallback)(void));
};




#endif


