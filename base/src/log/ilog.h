#ifndef CIS_ENGINE_ILOG_H
#define CIS_ENGINE_ILOG_H

#include <string>

namespace wolf
{
namespace log
{
class ilog
{
public:
  virtual ~ilog() {}
  enum logLevel
  {
    L_TRACE,
    L_DEBUG,
    L_INFO,
    L_WARNING,
    L_ERROR,
    L_FATAL,
  };

  virtual bool doLog(logLevel level, const std::string &msg) = 0;

protected:
  inline logLevel getLevel(int32_t level)
  {
    switch (level)
    {
    case 0:
      return logLevel::L_TRACE;
    case 1:
      return logLevel::L_DEBUG;
    case 2:
      return logLevel::L_INFO;
    case 3:
      return logLevel::L_WARNING;
    case 4:
      return logLevel::L_ERROR;
    case 5:
      return logLevel::L_FATAL;
    default:
      return logLevel::L_TRACE;
    }
  }

protected:
};
} // namespace log
} // namespace wolf

#endif