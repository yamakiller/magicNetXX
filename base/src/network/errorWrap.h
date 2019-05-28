#ifndef WOLF_NETWORK_ERRORWRAP_H
#define WOLF_NETWORK_ERRORWRAP_H

#include <stdint.h>

namespace wolf {
namespace network {
class errorWrap {
public:
  static int32_t lasterror();
  static int32_t wsalasterror();
};
} // namespace network
} // namespace wolf
#endif