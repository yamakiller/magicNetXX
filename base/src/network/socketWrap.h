#ifndef CIS_ENGINE_NETWORK_SOCKET_H
#define CIS_ENGINE_NETWORK_SOCKET_H

#include "platform.h"

#ifdef UT_PLATFORM_WINDOWS
#include <Mswsock.h>
#include <WinSock2.h>
#include <mstcpip.h>

#endif

#ifdef UT_PLATFORM_LINUX
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <unistd.h>

#define SOCKET_ERROR (-1)
#define INVALID_SOCKET 0
#endif

#define MAX_SOCKET_P 16
#define MAX_SOCKET (1 << MAX_SOCKET_P)

/*HANDLE TO SLOT INDEX*/
#define SOCKET_GET(handle) (((unsigned)handle) % MAX_SOCKET)
/*HANDLE HIGHT 16 bit ==> LOW 16 bit*/
#define SOCKET_HANDLE_TAG16(handle) ((handle >> MAX_SOCKET_P) & 0xffff)

#if (EAGAIN != EWOULDBLOCK)
#define AGAIN_WOULDBLOCK \
  EAGAIN:                \
  case EWOULDBLOCK
#else
#define AGAIN_WOULDBLOCK EAGAIN
#endif

#define WARNING_SIZE (1024 * 1024)

namespace engine
{
namespace network
{

#ifdef UT_PLATFORM_WINDOWS
typedef SOCKET wsocket_t;
typedef int32_t socklen_t;
#elif defined(UT_PLATFORM_LINUX)
typedef int wsocket_t;
#endif

enum class socketProtocol
{
  TCP = 0,
  UDP = 1,
  UDPv6 = 2,
  UNKNOWN = 255,
};

enum class socketState
{
  INVALID = 0,    //未被分配
  RESERVE = 1,    //已被分配
  PLISTEN = 2,    // 已监听未准备好
  LISTEN = 3,     // 已处于监听状态
  CONNECTING = 4, //连接中
  CONNECTED = 5,  //已经连接
  HALFCLOSE = 6,  //处于关闭状态
  PACCEPT = 7,    //接受得连接
  BIND = 8,       // Bind状态
};

enum class socketMessageType
{
  M_SOCKET_DATA = 1,
  M_SOCKET_START,
  M_SOCKET_CLOSE,
  M_SOCKET_ACCEPT,
  M_SOCKET_ERROR,
  M_SOCKET_EXIT,
  M_SOCKET_UDP,
  M_SOCKET_WARNING,
  M_SOCKET_MAX,
};

union socketAddr {
  struct sockaddr s;
  struct sockaddr_in v4;
  struct sockaddr_in6 v6;
};

class socketWrap
{
public:
  static int32_t init();
  static int32_t cleanup();

  static int32_t netAddr(const char *ip);
  static int16_t netPort(int16_t port);

  static wsocket_t socket(int family, int type, int proto);
  static wsocket_t invalid();
  static int32_t close(wsocket_t sock);

  static int32_t shutdown(wsocket_t sock);
  static int32_t setNonblock(wsocket_t sock);
  static int32_t bind(wsocket_t sock, const sockaddr *addr, socklen_t len);
  static int32_t connect(wsocket_t sock, const sockaddr *addr, socklen_t len);

  static int32_t listen(wsocket_t sock, int32_t que);
  static wsocket_t accept(wsocket_t sock, sockaddr *addr, socklen_t *len);
  static int32_t isvalidate(wsocket_t sock);

  static int32_t setSendbufSize(wsocket_t sock, int32_t size);
  static int32_t setRecvbufSize(wsocket_t sock, int32_t size);
  static int32_t setKeepalive(wsocket_t sock);
  static int32_t getAddrInfo(const char *addr, const char *strPort, const addrinfo *req, addrinfo **rest);
  static void freeAddrInfo(void *p);
  static const char *inetNtop(int32_t af, const void *cp, char *buf, socklen_t len);
  static int32_t lasterror();

  static int32_t send(wsocket_t sock, const char *data, int32_t size);
  static int32_t recv(wsocket_t sock, char *data, int32_t size);
  static int32_t sendto(wsocket_t sock, int32_t ip, int32_t port,
                        const char *data, int32_t size);
  static int32_t recvfrom(wsocket_t sock, int32_t *ip, int32_t *port,
                          char *data, int32_t size);

  static int32_t reuseaddr(wsocket_t sock);
};
} // namespace network
} // namespace engine

#endif