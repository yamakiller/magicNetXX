#ifndef MODULE_UNETADDR_H
#define MODULE_UNETADDR_H

#include <arpa/inet.h>
#include <stdint.h>
#include <string>

namespace cis {
namespace network {
class unetaddr {
public:
  unetaddr(int family = AF_INET, const std::string addr = "0.0.0.0",
           const uint16_t port = 21);
  unetaddr(int family = AF_INET, const uint8_t *addr = NULL,
           const uint16_t port = 21);

  const char *getAddress();
  const uint16_t getPort();

private:
  sockaddr m_addr;
  uint16_t m_port;
};
} // namespace network
} // namespace cis

#endif