#include "logFile.h"
#include "base.h"
#include "config.h"

#include "spdlog/sinks/daily_file_sink.h"

#define CONSOLE_FILE_LOGGER "daily_logger"

NS_CC_L_BEGIN

logFile::logFile() {
  auto daily_logger = spdlog::daily_logger_mt(
      CONSOLE_FILE_LOGGER, INSTGET_VAR(OPT, _logPath), 24, 0);
  daily_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  daily_logger->set_level(toLevel(getLevel(INSTGET_VAR(OPT, _logLevel))));

  spdlog::set_error_handler([](const std::string &msg) {
    fprintf(stderr, "log console error:%s\n", msg.c_str());
  });
}

logFile::~logFile() { spdlog::drop(CONSOLE_FILE_LOGGER); }

spdlog::level::level_enum logFile::toLevel(logLevel level) {
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

bool logFile::doLog(ilog::logLevel level, const std::string &msg) {
  switch (level) {
  case logLevel::L_TRACE:
    spdlog::get(CONSOLE_FILE_LOGGER)->trace(msg);
    break;
  case logLevel::L_DEBUG:
    spdlog::get(CONSOLE_FILE_LOGGER)->debug(msg.c_str());
    break;
  case logLevel::L_INFO:
    spdlog::get(CONSOLE_FILE_LOGGER)->info(msg.c_str());
    break;
  case logLevel::L_WARNING:
    spdlog::get(CONSOLE_FILE_LOGGER)->warn(msg.c_str());
    break;
  case logLevel::L_ERROR:
    spdlog::get(CONSOLE_FILE_LOGGER)->error(msg.c_str());
    break;
  case logLevel::L_FATAL:
    spdlog::get(CONSOLE_FILE_LOGGER)->critical(msg.c_str());
    break;
  default:
    spdlog::get(CONSOLE_FILE_LOGGER)->trace(msg.c_str());
    break;
  }
  return true;
}

NS_CC_L_END