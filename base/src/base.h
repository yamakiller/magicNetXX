#ifndef WOLF_BASE_H
#define WOLF_BASE_H

#include "format.h"
#include "log/logSystem.h"
#include "platform.h"
#include <algorithm>
#include <assert.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT(_0, _1, _2, _3, _4, _5, _6, _7, _8, \
                                          _9, N, ...)                         \
  N
#define PRIVATE_MACRO_VAR_ARGS_IMPL(args) PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT args
#define COUNT_MACRO_VAR_ARGS(...) \
  PRIVATE_MACRO_VAR_ARGS_IMPL((__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define PRIVATE_MACRO_CHOOSE_HELPER2(M, count) M##count
#define PRIVATE_MACRO_CHOOSE_HELPER1(M, count) \
  PRIVATE_MACRO_CHOOSE_HELPER2(M, count)
#define PRIVATE_MACRO_CHOOSE_HELPER(M, count) \
  PRIVATE_MACRO_CHOOSE_HELPER1(M, count)

#define INST_8(className, funName, ar1, ar2, ar3, ar4, ar5, ar6, ar7, ar8) \
  className::instance()->funName(ar1, ar2, ar3, ar4, ar5, ar6, ar7, ar8)
#define INST_7(className, funName, ar1, ar2, ar3, ar4, ar5, ar6, ar7) \
  className::instance()->funName(ar1, ar2, ar3, ar4, ar5, ar6, ar7)
#define INST_6(className, funName, ar1, ar2, ar3, ar4, ar5, ar6) \
  className::instance()->funName(ar1, ar2, ar3, ar4, ar5, ar6)
#define INST_5(className, funName, ar1, ar2, ar3, ar4, ar5) \
  className::instance()->funName(ar1, ar2, ar3, ar4, ar5)
#define INST_4(className, funName, ar1, ar2, ar3, ar4) \
  className::instance()->funName(ar1, ar2, ar3, ar4)
#define INST_3(className, funName, ar1, ar2, ar3) \
  className::instance()->funName(ar1, ar2, ar3)
#define INST_2(className, funName, ar1, ar2) \
  className::instance()->funName(ar1, ar2)
#define INST_1(className, funName, ar1) className::instance()->funName(ar1)
#define INST_0(className, funName) className::instance()->funName()
#define INST(className, funName, ...)                                   \
  PRIVATE_MACRO_CHOOSE_HELPER(INST_, COUNT_MACRO_VAR_ARGS(__VA_ARGS__)) \
  (className, funName, __VA_ARGS__)
#define INSTGET(className) className::instance()
#define INSTGET_VAR(className, var) INSTGET(className)->var

#define SYSLOG0(SOURCE, LEVEL, FMT, ...)                                \
  do                                                                    \
  {                                                                     \
    std::string body;                                                   \
    std::string header;                                                 \
    try                                                                 \
    {                                                                   \
      header = fmt::format("[:{:08x}] ", SOURCE);                       \
      body = fmt::format(FMT);                                          \
    }                                                                   \
    catch (...)                                                         \
    {                                                                   \
      header = "[:00000000] ";                                          \
      body = "format error";                                            \
    }                                                                   \
    std::string tail =                                                  \
        fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__);   \
    INST(wolf::log::logSystem, doLog, wolf::log::ilog::logLevel::LEVEL, \
         header + body + tail);                                         \
  } while (false)

#define SYSLOG(SOURCE, LEVEL, ...)                                      \
  do                                                                    \
  {                                                                     \
    std::string body;                                                   \
    std::string header;                                                 \
    try                                                                 \
    {                                                                   \
      header = fmt::format("[:{:08x}] ", SOURCE);                       \
      body = fmt::format(__VA_ARGS__);                                  \
    }                                                                   \
    catch (...)                                                         \
    {                                                                   \
      SYSLOG0(SOURCE, LEVEL, __VA_ARGS__);                              \
    }                                                                   \
    std::string tail = "";                                              \
    INST(wolf::log::logSystem, doLog, wolf::log::ilog::logLevel::LEVEL, \
         header + body + tail);                                         \
  } while (false)

#define SYSLOG_TRACE(SOURCE, ...) SYSLOG(SOURCE, L_TRACE, __VA_ARGS__)
#define SYSLOG_DEBUG(SOURCE, ...) SYSLOG(SOURCE, L_DEBUG, __VA_ARGS__)
#define SYSLOG_INFO(SOURCE, ...) SYSLOG(SOURCE, L_INFO, __VA_ARGS__)
#define SYSLOG_WARNING(SOURCE, ...) SYSLOG(SOURCE, L_WARNING, __VA_ARGS__)
#define SYSLOG_ERROR(SOURCE, ...) SYSLOG(SOURCE, L_ERROR, __VA_ARGS__)
#define SYSLOG_FATAL(SOURCE, ...) SYSLOG(SOURCE, L_FATAL, __VA_ARGS__)

#define LOCAL_LOG_TRACE(...) SYSLOG_TRACE(getHandle(), __VA_ARGS__)
#define LOCAL_LOG_DEBUG(...) SYSLOG_DEBUG(getHandle(), __VA_ARGS__)
#define LOCAL_LOG_INFO(...) SYSLOG_INFO(getHandle(), __VA_ARGS__)
#define LOCAL_LOG_WARNING(...) SYSLOG_WARNING(getHandle(), __VA_ARGS__)
#define LOCAL_LOG_ERROR(...) SYSLOG_ERROR(getHandle(), __VA_ARGS__)
#define LOCAL_LOG_FATAL(...) SYSLOG_FATAL(getHandle(), __VA_ARGS__)


NS_CC_BEGIN

template <typename T>
using atomic_t = std::atomic<T>;

extern std::mutex gDbgLock;
//写入debug打印
NS_CC_END

#endif