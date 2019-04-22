#ifndef MODULE_NET_USOCKET_H
#define MODULE_NET_USOCKET_H

#include "unet.h"
#include <base.h>
#include <stdint.h>

namespace cis {
namespace network {
#if defined(U_PLATFORM_APPLE)
typedef int32_t u_sock_t;
#elif U_PLATFORM_WINDOWS
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
  static int32_t setKleepAlive(u_sock_t s);
  static int32_t setSendBuffSize(u_sock_t s, size_t size);
  static int32_t setRecvBuffSize(u_sock_t s, size_t size);
  static int32_t setReuseAddr(u_sock_t s);
  static int32_t getPeerInfo(u_sock_t s, int32_t *outAddr, int32_t *outPort);
  static int32_t getSockInfo(u_sock_t s, int32_t *outAddr, int32_t *outPort);

  static int32_t isValidate(u_sock_t s);

  static int32_t bind(u_sock_t s, const char *addr, const int32_t port);
  static int32_t bind(u_sock_t s, const int32_t addr, const int32_t port);

  static int32_t connect(u_sock_t s, const char *addr, const int32_t port);
  static int32_t connect(u_sock_t s, const int32_t addr, const int32_t port);

  static int32_t listen(u_sock_t s, const int32_t que);

  static u_sock_t accept(u_sock_t s);

  static int32_t send(u_sock_t s, const char *inData, size_t size);
  static int32_t sendto(u_sock_t s, int32_t ip, int32_t port,
                        const char *inData, size_t size);

  static int32_t recv(u_sock_t s, char *outData, size_t size);
  static int32_t recvfrom(u_sock_t s, int32_t *outAddr, int32_t *outPort,
                          char *outData, size_t size);

  static int invalid();
};
} // namespace network
} // namespace cis
#endif
