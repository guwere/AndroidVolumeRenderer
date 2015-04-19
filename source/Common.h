#ifndef COMMON_H
#define COMMON_H
#pragma once

#pragma region Headers

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#ifdef ANDROID_NDK
//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>
#include <sys/time.h> // for getting the current time

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
//#include <KHR/khrplatform.h>

#include <android/sensor.h>
#include <android/log.h>
#include "android_native_app_glue.h"


#elif WIN32
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif

#pragma endregion

#pragma region EnumsANDMisc
enum Direction{FORWARD, BACKWARD, LEFT, RIGHT, NO_DIRECTION/*FORWARD_LEFT, FORWARD_RIGHT, BACKWARD_LEFT, BACKWARD_RIGHT*/};

#ifdef ANDROID_NDK
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#elif WIN32
#define LOGI(...) ((void) printf(__VA_ARGS__))
#endif 
#pragma endregion

#pragma region ApplicationParameters
#ifdef ANDROID_NDK

#elif WIN32

const int OPENGL_VERSION_MAJOR  = 3;
const int OPENGL_VERSION_MINOR = 1;
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
#endif

#define APP_NAME std::string("VolumeRenderer")
#define CLEAR_COLOR glm::vec4(0.46f, 0.53f, 0.6f, 1.0f)
#define CLEAR_MASK (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
#define INITIAL_CAMERA_POS glm::vec3(0.0f, 0.5f, 5.0f)
#define MOUSE_MOV_SPEED 5.0f
#define MOUSE_SENSITIVITY 0.25f
#define CAMERA_ZOOM 45.0f
#define CAMERA_FOV  45.0f
#define NEAR_CLIP_PLANE 0.001f
#define FAR_CLIP_PLANE 1000.0f
//const glm::mat4 PROJECTION = glm::perspective(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.0001f, 1000.0f);
//const glm::mat4 projection = glm::ortho(SCREEN_WIDTH/2, SCREEN_WIDTH, SCREEN_HEIGHT/2, SCREEN_HEIGHT);
#define MESH_SCALE 2.0f
#define MAX_RAY_STEPS 1000
#define RAY_STEP_SIZE 0.005f
#define GRADIENT_STEP_SIZE 0.005f
#define LIGHT_POS glm::vec3(-2.0f, 2.0f, 2.0f)

#define VOLUME_NAME std::string("CT-Knee.mhd")
//const std::string VOLUME_NAME = "backpack8.mhd";
//const std::string VOLUME_NAME = "vertebra8.mhd";
//const std::string VOLUME_NAME = "supine8.mhd";
#define TRANSFER_FN_NAME std::string("CT-Knee.tfi")
//const std::string TRANSFER_FN_NAME = "backpack8.tfi";
//const std::string TRANSFER_FN_NAME = "vertebra8.tfi";
//const std::string TRANSFER_FN_NAME = "supine8.tfi";
//const std::string TRANSFER_FN_NAME = "supine8_3.tfi";


#pragma endregion

#endif