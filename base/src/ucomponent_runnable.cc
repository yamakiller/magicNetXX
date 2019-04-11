#include "ucomponent_runnable.h"
#include "uwork_group.h"
#include "umodule.h"
#include <assert.h>
#include <functional>

namespace cis
{

int ucomponent_runnable::initialize(void *lpMod, const char *strParam)
{
    INST(uwork_group, append, std::bind(&ucomponent_runnable::run, this, std::placeholders::_1), lpMod);
    return 0;
}

void ucomponent_runnable::finalize()
{
}

void *ucomponent_runnable::run(void *parm)
{
    umodule *lpmodule = static_cast<umodule *>(parm);
    assert(lpmodule);
    while (!uwork_group::instance()->m_iShutdown)
    {
        if (runOnce() == -1)
            break;
    }
    return 0L;
}

} // namespace cis