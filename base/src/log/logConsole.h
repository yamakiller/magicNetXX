#ifndef WOLF_LOGCONSOLE_H
#define WOLF_LOGCONSOLE_H

#include "ilog.h"
#include <spdlog/spdlog.h>

namespace wolf {
namespace log {

class logConsole : public ilog {
public:
  logConsole();
  virtual ~logConsole();
  bool doLog(ilog::logLevel level, const std::string &msg);

private:
  spdlog::level::level_enum toLevel(logLevel level);
};

} // namespace log
} // namespace wolf

#endif