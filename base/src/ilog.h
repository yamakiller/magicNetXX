#ifndef CIS_ENGINE_ILOG_H
#define CIS_ENGINE_ILOG_H

#include <string>

namespace cis 
{
  class ilog
  {
  public:
      virtual ~ilog(){}
      enum class LogLevel
		{
			L_TRACE,
			L_DEBUG,
			L_NOTICE,
			L_WARNING,
			L_ERROR,
			L_FATAL,
		};

        virtual bool doLog(LogLevel level, uint32_t source, const std::string& msg) = 0;
  };
}


#endif