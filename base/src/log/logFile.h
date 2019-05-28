#ifndef WOLF_LOG_FILE_H
#define WOLF_LOG_FILE_H

#include "ilog.h"
#include <spdlog/spdlog.h>

namespace wolf
{
namespace log
{
class logFile : public ilog
{
public:
    logFile();
    virtual ~logFile();
    bool doLog(ilog::logLevel level, const std::string &msg);

private:
    spdlog::level::level_enum toLevel(logLevel level);
};
} // namespace log
} // namespace wolf

#endif