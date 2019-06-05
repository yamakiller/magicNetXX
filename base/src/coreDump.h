#ifndef WOLF_COREDUMP_H
#define WOLF_COREDUMP_H

#include "util/singleton.h"
#include <base.h>

namespace wolf
{
class coreDump : public util::singleton<coreDump>
{
public:
    coreDump() = default;
    virtual ~coreDump() = default;

public:
    void doListen();

private:
    void outStack(const char *sig);

private:
    static void coreSignal(int signal);
};
} // namespace wolf
#endif