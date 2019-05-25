#include "logConsole.h"
#include "base.h"
#include "config.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace engine {
namespace log {
logConsole::logConsole() {
  auto console = spdlog::stdout_color_mt("console");

  console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  console->set_level(
      toLevel(getLevel(INSTGET_VAR(coroutineOptions, _logLevel))));

  spdlog::set_error_handler([](const std::string &msg) {
    fprintf(stderr, "log console error:%s\n", msg.c_str());
  });
}

logConsole::~logConsole() {
  spdlog::drop("console");
  // m_console = nullptr;
}

spdlog::level::level_enum logConsole::toLevel(logLevel level) {
  switch (level) {
  case logLevel::L_TRACE:
    return spdlog::level::trace;
  case logLevel::L_DEBUG:
    return spdlog::level::debug;
  case logLevel::L_INFO:
    return spdlog::level::info;
  case logLevel::L_WARNING:
    return spdlog::level::warn;
  case logLevel::L_ERROR:
    return spdlog::level::err;
  case logLevel::L_FATAL:
    return spdlog::level::critical;
  default:
    return spdlog::level::trace;
  }
}

bool logConsole::doLog(ilog::logLevel level, const std::string &msg) {
  switch (level) {
  case logLevel::L_TRACE:
    spdlog::get("console")->trace(msg);
    break;
  case logLevel::L_DEBUG:
    spdlog::get("console")->debug(msg.c_str());
    break;
  case logLevel::L_INFO:
    spdlog::get("console")->info(msg.c_str());
    break;
  case logLevel::L_WARNING:
    spdlog::get("console")->warn(msg.c_str());
    break;
  case logLevel::L_ERROR:
    spdlog::get("console")->error(msg.c_str());
    break;
  case logLevel::L_FATAL:
    spdlog::get("console")->critical(msg.c_str());
    break;
  default:
    spdlog::get("console")->trace(msg.c_str());
    break;
  }
  return true;
}
} // namespace log
} // namespace engine