#ifndef CIS_ENGINE_ULOCK_H
#define CIS_ENGINE_ULOCK_H

#include <pthread.h>

namespace cis
{
class ulock
{
public:
    ulock()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }

    ~ulock()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

    bool trylock()
    {
        if (pthread_mutex_trylock(&m_mutex) == 0)
        {
            return true;
        }
        return false;
    }

private:
    pthread_mutex_t m_mutex;
};

class ulocking
{
public:
    ulocking(ulock *l) : m_lpLock(l)
    {
        m_lpLock->lock();
    }

    ~ulocking()
    {
        m_lpLock->unlock();
    }

private:
    ulock *m_lpLock;
};

} // namespace cis

#endif