#include "uframework.h"
#include "umodule.h"
#include "atomic.h"
#include "uexception.h"
#include "ilog.h"
#include "uwork_group.h"

#include <string.h>
#include <chrono>
#include <functional>

namespace cis
{

static uframework *sInstance = nullptr;

uframework::uframework()
{
    sInstance = this;
}

uframework::~uframework()
{
}

uframework *uframework::instance()
{
    return sInstance;
}

int uframework::initialize()
{
    INST(uwork_group, initialize);
    INST(umodule_mgr, initialize);

    
    return 0;
}

void uframework::finalize()
{
}

void uframework::startLoop()
{
    while (!INSTGET(uwork_group)->m_iShutdown)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    INST(umodule_mgr, finalize);
    INST(uwork_group, wait);
}

} // namespace cis