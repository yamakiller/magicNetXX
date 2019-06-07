#ifndef WOLF_NETWORK_ERRORWRAP_H
#define WOLF_NETWORK_ERRORWRAP_H

#include "platform.h"
#include <stdint.h>

NS_CC_N_BEGIN

class errorWrap {
public:
  static int32_t lasterror();
  static int32_t wsalasterror();
};

NS_CC_N_END

#endif