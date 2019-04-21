#ifndef MODULE_USOCKET_H
#define MODULE_USOCKET_H

#include <base.h>
#include <stdint.h>

namespace cis {
namespace network {
#if defined(UT_PLATFORM_APPLE)
typedef int32_t u_sock_t;
#elif UT_PLATFORM_WINDOWS
typedef int32_t u_sock_t;
#else
typedef int32_t u_sock_t;
#endif

class unet {
public:
  static int32_t apiInit();
  static void apiCleanup();
  static u_sock_t create(int32_t type, int32_t proto);
  static u_sock_t tcp();
  static u_sock_t udp();
  static int32_t close(u_sock_t s);
  static int32_t shutdown(u_sock_t s);
  static int32_t setNonblock(u_sock_t s);

  static int invalid();
};
} // namespace network
} // namespace cis
#endif
