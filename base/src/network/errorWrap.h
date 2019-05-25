#ifndef CIS_ENGINE_NETWORK_ERRORWRAP_H
#define CIS_ENGINE_NETWORK_ERRORWRAP_H

#include <stdint.h>

namespace engine {
namespace network {
class errorWrap {
public:
  static int32_t lasterror();
  static int32_t wsalasterror();
};
} // namespace network
} // namespace engine
#endif