#include "unetaddr.h"
#include <arpa/inet.h>
#include <string.h>

namespace cis {
namespace network {

unetaddr::unetaddr(int family, const std::string addr, const uint16_t port) {
  m_addr.sa_family = family;
  switch (m_addr.sa_family) {
  case AF_INET:
    sockaddr_in *sin = (sockaddr_in *)(&m_addr);
    inet_pton(AF_INET, addr.c_str(), &sin->sin_addr);
    sin->sin_port = port;
    break;
  case AF_INET6:
    sockaddr_in6 *sin6 = (sockaddr_in6 *)(&m_addr);
    inet_pton(AF_INET6, addr.c_str(), &sin6->sin6_addr);
    sin6->sin6_port = port;
    break;
  }
}

unetaddr::unetaddr(int family, const uint8_t *addr, const uint16_t port) {
  m_addr.sa_family = family;
  switch ((m_addr.sa_family)) {
  case AF_INET:
    sockaddr_in *sin = (sockaddr_in *)(&m_addr);
    memcpy(&sin->sin_addr.s_addr, addr, sizeof(sin->sin_addr.s_addr));
    sin->sin_port = port;
    break;
  case AF_INET6:
    sockaddr_in6 *sin6 = (sockaddr_in6 *)(&m_addr);
    memcpy(&sin6->sin6_addr.__u6_addr, addr, sizeof(sin6->sin6_addr.__u6_addr));
    sin6->sin6_port = port;
    break;
  }
}

const char *unetaddr::getAddress() {
  switch (m_addr.sa_family) {
  case AF_INET:
    char ipaddr[sizeof(struct in6_addr)];
    sockaddr_in sin;
    memcpy(&sin, &m_addr, sizeof(sin));
    inet_ntop(AF_INET, &sin.sin_addr, ipaddr, INET_ADDRSTRLEN);
    return ipaddr;

  case AF_INET6:
    char ipaddr[sizeof(struct in6_addr)];
    sockaddr_in6 sin6;
    memcpy(&sin6, &m_addr, sizeof(sin6));
    inet_ntop(AF_INET6, &sin6.sin6_addr, ipaddr, INET6_ADDRSTRLEN);
    return ipaddr;
  }

  return NULL;
}

const sockaddr_in *unetaddr::getAddressIn() { return (sockaddr_in *)&m_addr; }

const int32_t unetaddr::getPort() {
  switch ((m_addr.sa_family)) {
  case AF_INET:
    return ((sockaddr_in *)&m_addr)->sin_port;
  case AF_INET6:
    return ((sockaddr_in6 *)&m_addr)->sin6_port;
  }
  return 0;
}
} // namespace network
} // namespace cis