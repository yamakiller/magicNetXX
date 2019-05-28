#include "logSystem.h"
#include "logConsole.h"
#include "util/stringUtil.h"
#include <functional>

namespace wolf
{
namespace log
{

logSystem::logSystem() : m_log(nullptr) {}

int32_t logSystem::doStart()
{
  if (m_log != nullptr)
  {
    return -1;
  }

  m_log = new logConsole();
  return m_pid.doStart(std::bind(&logSystem::doDispatch, this));
}

void logSystem::doShutdown()
{
  m_pid.doShutdown();
  if (m_log != nullptr)
  {
    delete m_log;
    m_log = nullptr;
  }
}

ilog *logSystem::redirect(ilog *log)
{
  ilog *old = m_log;
  m_log = log;
  return old;
}

void logSystem::doDispatch()
{
  struct logMessage msg;
  while (m_mqs.pop(&msg))
  {
    m_log->doLog(static_cast<ilog::logLevel>(msg.level),
                 std::string(msg.message));
    util::memory::free(msg.message);
  }
}

void logSystem::doLog(ilog::logLevel level, const std::string &msg)
{

  if (m_log == nullptr)
  {
    fprintf(stderr, "log error\r\n");
    assert(false && "log error");
    return;
  }

  struct logMessage waitMsg;
  waitMsg.level = static_cast<int32_t>(level);
  waitMsg.message = util::stringUtil::strdup(msg.c_str());
  m_mqs.push(&waitMsg);
  m_pid.doPost();
}

void syslogTrace(const std::string &msg)
{
  INST(logSystem, doLog, ilog::logLevel::L_TRACE, msg);
}

void syslogDebug(const std::string &msg)
{
  INST(logSystem, doLog, ilog::logLevel::L_DEBUG, msg);
}

void syslogInfo(const std::string &msg)
{
  INST(logSystem, doLog, ilog::logLevel::L_INFO, msg);
}

void syslogWarning(const std::string &msg)
{
  INST(logSystem, doLog, ilog::logLevel::L_WARNING, msg);
}

void syslogError(const std::string &msg)
{
  INST(logSystem, doLog, ilog::logLevel::L_ERROR, msg);
}

void syslogFatal(const std::string &msg)
{
  INST(logSystem, doLog, ilog::logLevel::L_FATAL, msg);
}

} // namespace log
} // namespace wolf