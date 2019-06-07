#ifndef WOLF_SYSLOG_H
#define WOLF_SYSLOG_H

#include "ilog.h"
#include "util/memory.h"
#include "util/post.h"
#include "util/queue.h"
#include "util/singleton.h"
#include <thread>

NS_CC_L_BEGIN

class logSystem : public util::singleton<logSystem> {
  struct logMessage {
    int32_t level;
    char *message;
  };

  class logQueue : public util::queue<logMessage> {
  protected:
    void local_dropevent(logMessage *val) { util::memory::free(val->message); }
  };

public:
  logSystem();
  int32_t doStart();
  void doShutdown();
  ilog *redirect(ilog *log);
  void doLog(ilog::logLevel level, const std::string &msg);

private:
  void doDispatch();

private:
  ilog *m_log;
  logQueue m_mqs;
  util::post m_pid;
};

void syslogTrace(const std::string &msg);
void syslogDebug(const std::string &msg);
void syslogInfo(const std::string &msg);
void syslogWarning(const std::string &msg);
void syslogError(const std::string &msg);
void syslogFatal(const std::string &msg);

NS_CC_L_END

#endif