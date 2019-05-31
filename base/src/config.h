#ifndef WOLF_CONFIG_H
#define WOLF_CONFIG_H

#include "util/singleton.h"
#include <stdint.h>
#include <string>
#include <unordered_map>


namespace wolf {

struct coroutineOptions : public util::singleton<coroutineOptions> {

  uint64_t _debug = 0;
  uint32_t _thread = 6;
  uint32_t _actor_gcc_sleep_us = 1000;
  uint32_t _actor_gcc_timeout_us = 100 * 1000;
  uint32_t _stackSize = 1 * 1024 * 1024;
  uint32_t _single_timeout_us = 100 * 1000;
  uint32_t _dispatcher_thread_interval_us = 1000;

  const char *_componentPath = "./modules/?.so";

  size_t _logSize = 64;
  int32_t _logLevel = 0;
  const char *_logPath = nullptr;

public:
  bool load(std::string path);
  void unload();

  int getInt(std::string const &key, int defval);
  bool getBool(std::string const &key, bool defval);
  const char *getString(std::string const &key, const char *defval);

private:
  const char *getValue(std::string const &key);

private:
  std::unordered_map<std::string, std::string> m_env;
};

} // namespace wolf

#define OPT wolf::coroutineOptions

#endif