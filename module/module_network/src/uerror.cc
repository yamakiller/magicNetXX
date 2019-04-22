#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "uerror.h"
#include <base.h>

#ifdef U_PLATFORM_WINDOWS
#include <window.h>
#endif

namespace cis {
namespace network {
int32_t uerror::lasterror() {
#ifdef U_PLATFORM_WINDOWS
  return GetLastError();
#else
  return errno;
#endif
}

int32_t uerror::wsalasterror() {
#ifdef U_PLATFORM_WINDOWS
  return WSAGetLastError();
#else
  return errno;
#endif
}
} // namespace network
} // namespace cis