#ifndef WOLF_PLATFORM_H
#define WOLF_PLATFORM_H

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

#ifdef _MSC_VER
#define COMPILER_MICROSOFT 0
#define UT_COMPILER COMPILER_MICROSOFT
#elif defined(__BORLANDC__)
#define COMPILER_BORLAND 2
#define UT_COMPILER COMPILER_BORLAND
#elif defined(__INTEL_COMPILER)
#define COMPILER_INTEL 3
#define UT_COMPILER COMPILER_INTEL
#elif defined(__GNUC__)
#define COMPILER_GNU 1
#define UT_COMPILER COMPILER_GNU
#define UT_GCC_VERSION                                                         \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#error "FATAL ERROR: Unknown compiler."
#endif

#define NS_CC_BEGIN namespace wolf {

#define NS_CC_END }

#define NS_CC_U_BEGIN NS_CC_BEGIN namespace util {

#define NS_CC_U_END                                                            \
  NS_CC_END                                                                    \
  }

#define NS_CC_O_BEGIN NS_CC_BEGIN namespace operation {

#define NS_CC_O_END                                                            \
  NS_CC_END                                                                    \
  }

#define NS_CC_M_BEGIN NS_CC_BEGIN namespace module {

#define NS_CC_M_END                                                            \
  NS_CC_END                                                                    \
  }

#define NS_CC_A_BEGIN NS_CC_BEGIN namespace api {

#define NS_CC_A_END                                                            \
  NS_CC_END                                                                    \
  }

#define NS_CC_N_BEGIN NS_CC_BEGIN namespace network {

#define NS_CC_N_END                                                            \
  NS_CC_END                                                                    \
  }

#define NS_CC_L_BEGIN NS_CC_BEGIN namespace log {

#define NS_CC_L_END                                                            \
  NS_CC_END                                                                    \
  }

#if defined(__cplusplus) || defined(c_plusplus)
#define WOLF_C_API extern "C"
#else
#define WOLF_C_API extern
#endif

#endif