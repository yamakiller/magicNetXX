#ifndef CIS_ENGINE_CONFIG_H
#define CIS_ENGINE_CONFIG_H

#include "singleton.h"
#include <stdint.h>

namespace engine {

struct coroutineOptions : public singleton<coroutineOptions> {

  uint64_t _debug = 0;
  uint32_t _threadNum = 6;
  uint32_t _stackSize = 1 * 1024 * 1024;
  uint32_t _single_timeout_us = 100 * 1000;
  uint32_t _dispatcher_thread_interval_us = 1000;
};

} // namespace engine
#endif