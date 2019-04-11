#include "ucomponent_msg.h"
#include "umodule.h"

#include "uwork_group.h"

namespace cis
{
ucomponent_msg::ucomponent_msg() : m_lpMqs(NULL),
                                   m_sleep(0)
{
}

ucomponent_msg::~ucomponent_msg()
{
    RELEASE(m_lpMqs);
}

int ucomponent_msg::initialize(void *lpParam, const char *strParam)
{
    umodule *lpModule = static_cast<umodule *>(lpParam);
    assert(lpModule);
    m_lpMqs = new umq(lpModule->getId());
    assert(m_lpMqs);
    INST(uwork_group, append, std::bind(&ucomponent_msg::run, this, std::placeholders::_1), lpModule);

    return 0;
}

void ucomponent_msg::wakeup()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if (m_sleep > 0)
        m_cv.notify_one();
}

void *ucomponent_msg::run(void *parm)
{
    umodule *lpmodule = static_cast<umodule *>(parm);
    assert(lpmodule && m_lpMqs);

    umsg msg;
    while (!uwork_group::instance()->m_iShutdown)
    {
        if (m_lpMqs->pop(&msg))
        {
            int type = MSG_UNPACK_TYPE(msg.sz);
            size_t sz = MSG_UNPACK_SIZE(msg.sz);
            //time cost
            int r = runOnce(msg.source, msg.session, type, msg.data, sz);
            //time cost end
            if (!r)
            {
                umemory::free(msg.data);
            }
        }
        else
        {
            std::unique_lock<std::mutex> lck(m_mutex);
            ++m_sleep;
            if (!uwork_group::instance()->m_iShutdown)
                m_cv.wait(lck);
            --m_sleep;
        }
    }
    return 0L;
}

} // namespace cis