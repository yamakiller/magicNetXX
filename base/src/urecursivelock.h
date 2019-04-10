#ifndef CIS_ENGINE_URECURESIVELOCK_H
#define CIS_ENGINE_URECURESIVELOCK_H

#include "uspinlock.h"
#include <assert.h>

namespace cis
{
class urecuresivelock
{
public:
    urecuresivelock(uspinlock *ls) : m_lpLock(ls),
                                     m_icount(0)
    {
    }
    ~urecuresivelock() {}

    void lock()
    {
        if (m_icount == 0)
        {
            m_lpLock->lock();
        }
        ++m_icount;
    }

    int trylock()
    {
        if (m_icount == 0)
        {
            if (!m_lpLock->trylock())
                return 0;
        }
        ++m_icount;
        return 1;
    }

    void unlock()
    {
        --m_icount;
        if (m_icount <= 0)
        {
            assert(m_icount == 0);
            m_lpLock->unlock();
        }
    }

private:
    uspinlock *m_lpLock;
    int m_icount;
};
} // namespace cis

#endif
