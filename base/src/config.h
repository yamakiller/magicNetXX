#ifndef CIS_ENGINE_CONFIG_H
#define CIS_ENGINE_CONFIG_H

#include "util/singleton.h"
#include <stdint.h>
#include <string>

namespace engine {

struct coroutineOptions : public util::singleton<coroutineOptions> {

  uint64_t _debug = 0;
  uint32_t _threadNum = 6;
  uint32_t _stackSize = 1 * 1024 * 1024;
  uint32_t _single_timeout_us = 100 * 1000;
  uint32_t _dispatcher_thread_interval_us = 1000;

  size_t _logSize = 63;
  int32_t _logLevel = 0;
  const char *_logPath = nullptr;
};

} // namespace engine
#endif