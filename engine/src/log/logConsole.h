#ifndef WOLF_LOGCONSOLE_H
#define WOLF_LOGCONSOLE_H

#include "ilog.h"
#include <spdlog/spdlog.h>

NS_CC_L_BEGIN

class logConsole : public ilog {
public:
  logConsole();
  virtual ~logConsole();
  bool doLog(ilog::logLevel level, const std::string &msg);

private:
  spdlog::level::level_enum toLevel(logLevel level);
};

NS_CC_L_END

#endif