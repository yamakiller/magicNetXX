#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "errorWrap.h"
#include "platform.h"

#ifdef UT_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace wolf {
namespace network {

#ifdef UT_PLATFORM_WINDOWS
int32_t errorWrap::lasterror() { return GetLastError(); }
int32_t errorWrap::wsalasterror() { return WSAGetLastError(); }
#elif defined(UT_PLATFORM_LINUX)
int32_t errorWrap::lasterror() { return errno; }
int32_t errorWrap::wsalasterror() { return errno; }
#endif

} // namespace network
} // namespace wolf