#ifndef COMMON_H
#define COMMON_H
#pragma once

#pragma region Headers

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#ifdef ANDROID
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

#ifdef ANDROID
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#elif WIN32
#define LOGI(...) ((void) printf(__VA_ARGS__))
#endif 
#pragma endregion

#pragma region ApplicationParameters
#ifdef ANDROID

#elif WIN32

const int OPENGL_VERSION_MAJOR  = 3;
const int OPENGL_VERSION_MINOR = 1;
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
#endif

const std::string APP_NAME = "VolumeRenderer";
const glm::vec4 CLEAR_COLOR = glm::vec4(0.46f, 0.53f, 0.6f, 1.0f);
const GLbitfield CLEAR_MASK = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
const glm::vec3 INITIAL_CAMERA_POS = glm::vec3(0.0f, 1.0f, 3.0f);
const float CAMERA_FOV = 45.0f;
const float NEAR_CLIP_PLANE = 0.001f;
const float FAR_CLIP_PLANE = 1000.0f;
//const glm::mat4 PROJECTION = glm::perspective(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.0001f, 1000.0f);
//const glm::mat4 projection = glm::ortho(SCREEN_WIDTH/2, SCREEN_WIDTH, SCREEN_HEIGHT/2, SCREEN_HEIGHT);
const float MESH_SCALE = 4.0f;
const int MAX_RAY_STEPS = 1000;
const float RAY_STEP_SIZE = 0.005f;
const float GRADIENT_STEP_SIZE = 0.005f;
const glm::vec3 LIGHT_POS = glm::vec3(-2.0f, 2.0f, 2.0f);

const std::string VOLUME_NAME = "CTknee.mhd";
//const std::string VOLUME_NAME = "backpack8.mhd";
//const std::string VOLUME_NAME = "vertebra8.mhd";
//const std::string VOLUME_NAME = "supine8.mhd";
const std::string TRANSFER_FN_NAME = "CT-Knee.tfi";
//const std::string TRANSFER_FN_NAME = "backpack8.tfi";
//const std::string TRANSFER_FN_NAME = "vertebra8.tfi";
//const std::string TRANSFER_FN_NAME = "supine8.tfi";
//const std::string TRANSFER_FN_NAME = "supine8_3.tfi";


#pragma endregion

#endif