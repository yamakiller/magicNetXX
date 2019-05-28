#include "socketWrap.h"
#include "errorWrap.h"
#include <assert.h>
#include <string.h>

//#include <arpa/inet.h>
//#include <fcntl.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <unistd.h>

namespace wolf
{
namespace network
{

struct sockaddr_in ip4Addr(const char *ip, int32_t port)
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = socketWrap::netPort((int16_t)port);
  addr.sin_addr.s_addr = socketWrap::netAddr(ip);

  return addr;
}

int32_t socketWrap::init()
{
#ifdef UT_PLATFORM_WINDOWS
  WSADATA wsa_data;
  return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0 ? 0 : -ut_wsalasterror();
#endif
  return 0;
}

int32_t socketWrap::cleanup()
{
#ifdef UT_PLATFORM_WINDOWS
  return WSACleanup();
#endif
  return 0;
}

int32_t socketWrap::netAddr(const char *ip)
{
  if (ip == 0)
  {
    return 0;
  }
  else
  {
    return (int32_t)inet_addr(ip);
  }
}

int16_t socketWrap::netPort(int16_t port) { return htons(port); }

wsocket_t socketWrap::socket(int family, int type, int proto)
{
  return ::socket(family, type, proto);
}

wsocket_t socketWrap::invalid() { return INVALID_SOCKET; }

int32_t socketWrap::close(wsocket_t sock)
{
#ifdef UT_PLATFORM_WINDOWS
  return 0 == closesocket(sock) ? 0 : errorWrap::wsalasterror();
#elif defined(UT_PLATFORM_LINUX)
  return 0 == close(sock) ? 0 : errorWrap::wsalasterror();
#endif
}

int32_t socketWrap::shutdown(wsocket_t sock)
{
#ifdef UT_PLATFORM_WINDOWS
  return 0 == ::shutdown(sock, 2) ? 0 : errorWrap::wsalasterror();
#elif defined(UT_PLATFORM_LINUX)
  return 0 == ::shutdown(sock, SHUT_RDWR) ? 0 : errorWrap::wsalasterror();
#endif
}

int32_t socketWrap::setNonblock(wsocket_t sock)
{
#ifdef UT_PLATFORM_WINDOWS
  unsigned long ul = 1;
  return ioctlsocket(sock, FIONBIO, (unsigned long *)&ul) == 0
             ? 0
             : errorWrap::wsalasterror();
#elif defined(UT_PLATFORM_LINUX)
  int32_t err = -1;
  err = fcntl(sock, F_GETFL, 0);
  if (err == -1)
  {
    return errorWrap::wsalasterror();
  }
  err = fcntl(sock, F_SETFL, err | O_NONBLOCK);

  return (err == -1) ? errorWrap::wsalasterror() : 0;
#endif
}

int32_t socketWrap::bind(wsocket_t sock, const sockaddr *addr, socklen_t len)
{
  return ::bind(sock, addr, len) == 0 ? 0 : errorWrap::wsalasterror();
}

int32_t socketWrap::connect(wsocket_t sock, const sockaddr *addr, socklen_t len)
{
  return ::connect(sock, addr, len);
}

int32_t socketWrap::listen(wsocket_t sock, int32_t que)
{
  return listen(sock, que) == 0;
}

wsocket_t socketWrap::accept(wsocket_t sock, sockaddr *addr, socklen_t *len) { return ::accept(sock, addr, len); }

int32_t socketWrap::isvalidate(wsocket_t sock) { return sock != invalid(); }

int32_t socketWrap::setSendbufSize(wsocket_t sock, int32_t size)
{
  int32_t sizelen = sizeof(size);
  return setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizelen) == 0
             ? 0
             : errorWrap::wsalasterror();
}

int32_t socketWrap::setRecvbufSize(wsocket_t sock, int32_t size)
{
  int32_t sizelen = sizeof(size);
  return setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizelen) == 0
             ? 0
             : errorWrap::wsalasterror();
}

int32_t socketWrap::setKeepalive(wsocket_t sock)
{
#ifdef UT_PLATFORM_WINDOWS
  int32_t nerr = SOCKET_ERROR;
  struct tcp_keepalive alive_in = {0};
  struct tcp_keepalive alive_out = {0};
  unsigned long bytes_return = 0;
  int32_t nKeepAlive = 1;

  assert(sock != 0);

  if (0 != setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&nKeepAlive,
                      sizeof(nKeepAlive)))
  {
    return errorWrap::wsalasterror();
  }

  alive_in.keepalivetime = 5000;
  alive_in.keepaliveinterval = 2000;
  alive_in.onoff = TRUE;
  bytes_return = 0;

  nerr = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
                  &alive_out, sizeof(alive_out), &bytes_return, NULL, NULL);

FAIL:
  return nerr == 0 ? 0 : errorWrap::wsalasterror();
#elif defined(UT_PLATFORM_LINUX)
  int32_t keepalive = 1;
  int32_t err;
  err = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive,
                   sizeof(keepalive));
  if (0 != err)
  {
    return errorWrap::wsalasterror();
  }
  return 0;
#endif
}

int32_t socketWrap::getAddrInfo(const char *addr, const char *strPort, const addrinfo *req, addrinfo **rest)
{
  return getaddrinfo(addr, strPort, req, rest);
}

void socketWrap::freeAddrInfo(void *p)
{
#ifdef UT_PLATFORM_WINDOWS
  freeaddrinfo((PADDRINFOA)p);
#else
  freeaddrinfo((addrinfo *)p);
#endif
}

const char *socketWrap::inetNtop(int32_t af, const void *cp, char *buf, socklen_t len)
{
#ifdef UT_PLATFORM_WINDOWS
  return rtc_inet_ntop(af, cp, buf, len);
#else
  return inet_ntop(af, cp, buf, len);
#endif
}

/*int32_t socketWrap::getPeerInfo(wsocket_t sock, int32_t *ip, int32_t *port)
{
  struct sockaddr_in addr;
  socklen_t addrlen = 0;
  addrlen = sizeof(addr);
  memset(&addr, 0, addrlen);

  assert(ip != 0);
  assert(port != 0);

  *ip = 0;
  *port = 0;

  if (0 != getpeername(sock, (struct sockaddr *)&addr, &addrlen))
  {
    return errorWrap::wsalasterror();
  }

#ifdef UT_PLATFORM_WINDOWS
  *ip = addr.sin_addr.S_un.S_addr;
#else
  *ip = addr.sin_addr.s_addr;
#endif
  *port = ntohs(addr.sin_port);

  return 0;

FAIL:
  return -1;
}*/

/*int32_t socketWrap::getSockInfo(wsocket_t sock, int32_t *ip, int32_t *port)
{
  struct sockaddr_in addr;
  socklen_t addrlen = 0;
  addrlen = sizeof(addr);
  memset(&addr, 0, addrlen);

  assert(ip != 0);
  assert(port != 0);

  *ip = 0;
  *port = 0;

  if (0 != getsockname(sock, (struct sockaddr *)&addr, &addrlen))
  {
    return errorWrap::wsalasterror();
  }

#ifdef UT_PLATFORM_WINDOWS
  *ip = addr.sin_addr.S_un.S_addr;
#else
  *ip = addr.sin_addr.s_addr;
#endif
  *port = ntohs(addr.sin_port);

  return 0;

FAIL:
  return -1;
}*/

int32_t socketWrap::send(wsocket_t sock, const char *data, int32_t size)
{
  int32_t nerr = -1;

#ifdef UT_PLATFORM_LINUX
  nerr = ::send(sock, data, size, MSG_NOSIGNAL);
#endif
#ifdef UT_PLATFORM_WINDOWS
  nerr = ::send(sock, data, size, 0);
#endif

  return nerr;
}

int32_t socketWrap::recv(wsocket_t sock, char *data, int32_t size)
{
  int32_t nerr = -1;

  if (data == 0 || size <= 0)
  {
    return -1;
  }

  nerr = ::recv(sock, data, size, 0);

  return nerr;
}

int32_t socketWrap::sendto(wsocket_t sock, int32_t ip, int32_t port,
                           const char *data, int32_t size)
{
  struct sockaddr_in addr;
  int32_t nerr = -1;

  if (data == 0 || size <= 0)
  {
    return -1;
  }

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = ip;

#ifdef UT_PLATFORM_LINUX
  nerr = ::sendto(sock, data, size, MSG_NOSIGNAL, (struct sockaddr *)&addr,
                  sizeof(addr));
#endif
#ifdef UT_PLATFORM_WINDOWS
  nerr = ::sendto(sock, data, size, 0, (struct sockaddr *)&addr, sizeof(addr));
#endif

  return nerr;
}

int32_t socketWrap::recvfrom(wsocket_t sock, int32_t *ip, int32_t *port,
                             char *data, int32_t size)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  int32_t nerr = -1;

  if (data == 0 || size <= 0)
  {
    return -1;
  }

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = 0;

  nerr = ::recvfrom(sock, data, size, 0, (struct sockaddr *)&addr, &addrlen);

  if (ip != 0)
  {
    *ip = addr.sin_addr.s_addr;
  }
  if (port != 0)
  {
    *port = ntohs(addr.sin_port);
  }
  return nerr;
}

int32_t socketWrap::reuseaddr(wsocket_t sock)
{
  int32_t reuseaddr = 1;
  return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseaddr,
                    sizeof(reuseaddr));
}

} // namespace network
} // namespace wolf