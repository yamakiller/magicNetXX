#include "usocket.h"

#if defined(UT_PLATFORM_LINUX) || defined(UT_PLATFORM_APPLE)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef UT_PLATFORM_WINDOWS
#include <Mswsock.h>
#include <WinSock2.h>
#include <mstcpip.h>
typedef int32_t socklen_t;
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

namespace cis {
namespace network {
int32_t unet::apiInit() {
#ifdef UT_PLATFORM_WINDOWS
  WSADATA wsa_data;
  return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0 ? 0 : -ut_wsalasterror();
#else
  return 0;
#endif
}

void unet::apiCleanup() {
#ifdef UT_PLATFORM_WINDOWS
  WSACleanup();
#else
#endif // DEBUG
}

u_sock_t unet::create(int32_t type, int32_t proto) {
  return ::socket(AF_INET, type, proto);
}

u_sock_t unet::tcp() { return unet::create(SOCK_STREAM, IPPROTO_TCP); }

u_sock_t unet::udp() { return unet::create(SOCK_DGRAM, IPPROTO_UDP); }

int unet::invalid() { return INVALID_SOCKET; }

int32_t unet::close(u_sock_t s) {
#ifdef UT_PLATFORM_WINDOWS
  return 0 == closesocket(s) ? 0 : -1;
#else
  return 0 == close(s) ? 0 : -1;
#endif
}

int32_t unet::shutdown(u_sock_t s) {
#ifdef UT_PLATFORM_WINDOWS
  return shutdown(s, 2) == 0 ? 0 : -1;
#else
  return shutdown(s, SHUT_RDWR) == 0 ? 0 : -1;
#endif
}

int32_t unet::setNonblock(u_sock_t s) {
#ifdef UT_PLATFORM_WINDOWS
  unsigned long ul = 1;
  return ioctlsocket(s, FIONBIO, (unsigned long *)&ul) == 0 ? 0 : -1;
#else
  int32_t err = -1;
  err = fcntl(s, F_GETFL, 0);
  if (err == -1) {
    return -1;
  }
  err = fcntl(s, F_SETFL, err | O_NONBLOCK);

  return (err == -1) ? -1 : 0;
#endif
}
} // namespace network
} // namespace cis