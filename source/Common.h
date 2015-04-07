#ifndef COMMON_H
#define COMMON_H
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>


#define APP_NAME "VolumeRenderer"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum Direction{FORWARD, BACKWARD, LEFT, RIGHT, NO_DIRECTION/*FORWARD_LEFT, FORWARD_RIGHT, BACKWARD_LEFT, BACKWARD_RIGHT*/};

#ifdef ANDROID
//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
//#include <KHR/khrplatform.h>

#include <android/sensor.h>
#include <android/log.h>
#include "android_native_app_glue.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

#elif WIN32
#include <stdio.h>
#define LOGI(...) ((void) printf(__VA_ARGS__))

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 1

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800

#endif // ANDROID




#endif