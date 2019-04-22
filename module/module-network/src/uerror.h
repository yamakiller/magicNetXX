#ifndef MODULE_NET_UERROR_H
#define MODULE_NET_UERROR_H
#include <stdint.h>

namespace cis {
namespace network {
class uerror {
public:
  static int32_t lasterror();
  static int32_t wsalasterror();
};
} // namespace network
} // namespace cis
#endif // !1 MODULE_NET_UERROR_H