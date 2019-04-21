#ifndef CIS_ENGINE_BASE_H
#define CIS_ENGINE_BASE_H

#include "ilog.h"
#include "ucomponent_msg.h"
#include "uexception.h"
#include "uframework.h"
#include "umodule.h"


#if defined(_WIN64)
#define UT_PLATFORM_WINDOWS 0
#define UT_PLATFORM PLATFORM_WINDOWS
#elif defined(__WIN32__) || defined(WIN32) || defined(_WIN32)
#define UT_PLATFORM_WINDOWS 0
#define UT_PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE_CC__)
#define UT_PLATFORM_APPLE 2
#define UT_PLATFORM PLATFORM_APPLE
#elif defined(__INTEL_COMPILER)
#define UT_PLATFORM_INTEL 3
#define UT_PLATFORM PLATFORM_INTEL
#else
#define UT_PLATFORM_LINUX 1
#define UT_PLATFORM UT_PLATFORM_LINUX
#endif

#endif