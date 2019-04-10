#ifndef CIS_ENGINE_URWLOCK_H
#define CIS_ENGINE_URWLOCK_H

#include <pthread.h>

namespace cis
{
class urwlock
{
public:
  urwlock()
  {
    pthread_rwlock_init(&m_mutex, NULL);
  }

  ~urwlock()
  {
    pthread_rwlock_destroy(&m_mutex);
  }

  inline void wlock()
  {
    pthread_rwlock_wrlock(&m_mutex);
  }

  inline void rlock()
  {
    pthread_rwlock_rdlock(&m_mutex);
  }

  inline void unlock()
  {
    pthread_rwlock_unlock(&m_mutex);
  }

private:
  pthread_rwlock_t m_mutex;
};

class wlocking
{
public:
  wlocking(urwlock *h) : m_lpLock(h)
  {
    m_lpLock->wlock();
  }

  ~wlocking()
  {
    m_lpLock->unlock();
    m_lpLock = NULL;
  }

private:
  urwlock *m_lpLock;
};

class rlocking
{
public:
  rlocking(urwlock *h) : m_lpLock(h)
  {
    m_lpLock->rlock();
  }

  ~rlocking()
  {
    m_lpLock->unlock();
    m_lpLock = NULL;
  }

private:
  urwlock *m_lpLock;
};
} // namespace cis

#endif