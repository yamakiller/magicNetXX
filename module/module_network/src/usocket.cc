#include "usocket.h"
#include "uerror.h"
#include "unetaddr.h"

#if defined(UNET_PLATFORM_LINUX) || defined(UNET_PLATFORM_APPLE)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef UNET_PLATFORM_WINDOWS
#include <Mswsock.h>
#include <WinSock2.h>
#include <mstcpip.h>
typedef int32_t socklen_t;
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#if defined(UNET_PLATFORM_LINUX)
#define U_SOL_TCP SOL_TCP
#elif defined(UNET_PLATFORM_APPLE)
#define U_SOL_TCP SOL_SOCKET
#endif

namespace cis {
namespace network {
int32_t unet::apiInit() {
#ifdef UNET_PLATFORM_WINDOWS
  WSADATA wsa_data;
  return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0 ? 0
                                                    : -uerror::wsalasterror();
#else
  return 0;
#endif
}

void unet::apiCleanup() {
#ifdef U_PLATFORM_WINDOWS
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
#ifdef UNET_PLATFORM_WINDOWS
  return 0 == closesocket(s) ? 0 : -uerror::wsalasserror();
#else
  return 0 == close(s) ? 0 : -uerror::wsalasterror();
#endif
}

int32_t unet::shutdown(u_sock_t s) {
#ifdef UNET_PLATFORM_WINDOWS
  return ::shutdown(s, 2) == 0 ? 0 : -uerror::wsalasterror();
#else
  return ::shutdown(s, SHUT_RDWR) == 0 ? 0 : -uerror::wsalasterror();
#endif
}

int32_t unet::setNonblock(u_sock_t s) {
#ifdef UNET_PLATFORM_WINDOWS
  unsigned long ul = 1;
  return ioctlsocket(s, FIONBIO, (unsigned long *)&ul) == 0
             ? 0
             : uerror::wsalasterror();
#else
  int32_t err = -1;
  err = fcntl(s, F_GETFL, 0);
  if (err == -1) {
    return -uerror::wsalasterror();
  }
  err = fcntl(s, F_SETFL, err | O_NONBLOCK);

  return (err == -1) ? -uerror::wsalasterror() : 0;
#endif
}

int32_t unet::setKleepAlive(u_sock_t s) {
#ifdef UNET_PLATFORM_WINDOWS
  int32_t nerr = SOCKET_ERROR;
  struct tcp_keepalive alive_in = {0};
  struct tcp_keepalive alive_out = {0};
  unsigned long bytes_return = 0;
  int32_t nKeepAlive = 1;

  if (0 != setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&nKeepAlive,
                      sizeof(nKeepAlive))) {
    return -uerror::wsalasterror();
  }

  alive_in.keepalivetime = 5000;
  alive_in.keepaliveinterval = 2000;
  alive_in.onoff = TRUE;
  bytes_return = 0;

  nerr = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
                  &alive_out, sizeof(alive_out), &bytes_return, NULL, NULL);

  return nerr == 0 ? 0 : -uerror::wsalasterror()
#else
  int32_t keepalive = 1;
  int32_t keepidle = 120;
  int32_t keepinterval = 5;
  int32_t keepcount = 4;
  int32_t err = 0;

  err = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive,
                   sizeof(keepalive));
  if (0 != err) {
    return -uerror::wsalasterror();
  }

#ifdef UNET_PLATFORM_LINUX
  err = setsockopt(s, U_SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle,
                   sizeof(keepidle));
#endif

  if (0 != err) {
    return -uerror::wsalasterror();
  }
  err = setsockopt(s, U_SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval,
                   sizeof(keepinterval));
  if (0 != err) {
    return -uerror::wsalasterror();
  }

  err = setsockopt(s, U_SOL_TCP, TCP_KEEPCNT, (void *)&keepcount,
                   sizeof(keepcount));
  if (0 != err) {
    return -uerror::wsalasterror();
  }

  int32_t on = 1;
  err = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
  if (0 != err) {
    return -uerror::wsalasterror();
  }

  return 0;
#endif
}

int32_t unet::setReuseAddr(u_sock_t s) {
  int32_t reuseaddr = 1;
  return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseaddr,
                    sizeof(reuseaddr)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::setSendBuffSize(u_sock_t s, size_t size) {
  return setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::setRecvBuffSize(u_sock_t s, size_t size) {
  return setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::bind(u_sock_t s, const char *addr, const int32_t port) {
  unetaddr addr_in(AF_INET, addr, port);
  return ::bind(s, (struct sockaddr *)addr_in.getAddressIn(),
                sizeof(*addr_in.getAddressIn)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::bind(u_sock_t s, const int32_t addr, const int32_t port) {
  unetaddr addr_in(AF_INET, (const uint8_t *)&addr, port);
  return ::bind(s, (struct sockaddr *)addr_in.getAddressIn(),
                sizeof(*addr_in.getAddressIn)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::connect(u_sock_t s, const char *addr, const int32_t port) {
  unetaddr addr_in(AF_INET, addr, port);
  return ::connect(s, (struct sockaddr *)addr_in.getAddressIn(),
                   sizeof(*addr_in.getAddressIn)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::connect(u_sock_t s, const int32_t addr, const int32_t port) {
  unetaddr addr_in(AF_INET, (const uint8_t *)&addr, port);
  return ::connect(s, (struct sockaddr *)addr_in.getAddressIn(),
                   sizeof(*addr_in.getAddressIn)) == 0
             ? 0
             : -uerror::wsalasterror();
}

int32_t unet::listen(u_sock_t s, int32_t que) {
  return ::listen(s, que) == 0 ? 0 : -uerror::wsalasterror();
}

u_sock_t unet::accept(u_sock_t s) { return ::accept(s, 0, 0); }

int32_t unet::isValidate(u_sock_t s) { return s != unet::invalid(); }

int32_t unet::getPeerInfo(u_sock_t s, int32_t *outAddr, int32_t *outPort) {
  struct sockaddr_in addr;
  socklen_t addrlen = 0;
  addrlen = sizeof(addr);
  memset(&addr, 0, addrlen);

  *outAddr = 0;
  *outPort = 0;

  if (0 != getpeername(s, (struct sockaddr *)&addr, &addrlen)) {
    return -uerror::wsalasterror();
  }

#ifdef UNET_PLATFORM_WINDOWS
  *outAddr = addr.sin_addr.S_un.S_addr;
#else
  *outAddr = addr.sin_addr.s_addr;
#endif
  *outPort = ntohs(addr.sin_port);

  return 0;
}

int32_t unet::getSockInfo(u_sock_t s, int32_t *outAddr, int32_t *outPort) {
  struct sockaddr_in addr;
  socklen_t addrlen = 0;
  addrlen = sizeof(addr);
  memset(&addr, 0, addrlen);

  *outAddr = 0;
  *outPort = 0;

  if (0 != getsockname(s, (struct sockaddr *)&addr, &addrlen)) {
    return -uerror::wsalasterror();
  }

#ifdef UNET_PLATFORM_WINDOWS
  *outAddr = addr.sin_addr.S_un.S_addr;
#else
  *outAddr = addr.sin_addr.s_addr;
#endif
  *outPort = ntohs(addr.sin_port);

  return 0;
}

int32_t unet::send(u_sock_t s, const char *inData, size_t size) {
  int32_t nerr = -1;

#if defined(UNET_PLATFORM_LINUX)
  nerr = ::send(s, inData, size, MSG_NOSIGNAL);
#elif defined(UNET_PLATFORM_APPLE)
  nerr = ::send(s, inData, size, 0);
#else
  nerr = ::send(s, inData, size, 0);
#endif

  return nerr >= 0 ? nerr : -uerror::wsalasterror();
}

int32_t unet::sendto(u_sock_t s, int32_t ip, int32_t port, const char *inData,
                     size_t size) {
  struct sockaddr_in addr;
  int32_t nerr = -1;

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = ip;

#if defined(UNET_PLATFORM_LINUX)
  nerr = ::sendto(s, inData, size, MSG_NOSIGNAL, (struct sockaddr *)&addr,
                  sizeof(addr));
#elif defined(UNET_PLATFORM_APPLE)
  nerr = ::sendto(s, inData, size, 0, (struct sockaddr *)&addr, sizeof(addr));
#else
  nerr = ::sendto(s, inData, size, 0, (struct sockaddr *)&addr, sizeof(addr));
#endif

  return nerr >= 0 ? nerr : -uerror::wsalasterror();
}

int32_t unet::recv(u_sock_t s, char *outData, size_t size) {
  int32_t nerr = -1;

  nerr = ::recv(s, outData, size, 0);

  return nerr >= 0 ? nerr : -uerror::wsalasterror();
}

int32_t unet::recvfrom(u_sock_t s, int32_t *outAddr, int32_t *outPort,
                       char *outData, size_t size) {
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  int32_t nerr = -1;

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = 0;

  nerr = ::recvfrom(s, outData, size, 0, (struct sockaddr *)&addr, &addrlen);

  if (outAddr != 0) {
    *outAddr = addr.sin_addr.s_addr;
  }
  if (outPort != 0) {
    *outPort = ntohs(addr.sin_port);
  }
  return nerr >= 0 ? nerr : -uerror::wsalasterror();
}

} // namespace network
} // namespace cis