#ifndef CIS_ENGINE_DEQUE_H
#define CIS_ENGINE_DEQUE_H

#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>

#include "spinlock.h"

namespace engine
{
template <class T, bool threadSafe>
class deque final
{
public:
    typedef typename std::conditional<threadSafe, spinlock, unspinlock>::type lock_t;
    typedef typename std::conditional<threadSafe, lock_guard<spinlock>, unslk_lock_guard>::type locked_guard;

public:
    deque() = default;
    ~deque() = default;

    //LIFO

    void pushWithoutLock(const T &val)
    {
        m_deque.push(val);
    }

    void push(const T &val)
    {
        locked_guard<lock_T> l(m_lock);
        pushWithoutLock(val);
    }

    void push(const std::list<T> &ls){
        std::move()}

    T popFront()
    {
        m_lock.lock();

        if (m_deque.empty())
        {
            m_lock.unlock();
            return nullptr;
        }

        T val = m_deque.front();
        m_deque.pop_front();

        m_lock.unlock();

        return val;
    }

    //for steal FIFO
    T popBack()
    {
        if (!m_lock.try_lock())
            return nullptr;

        if (m_deque.empty())
        {
            m_lock.unlock();
            return nullptr;
        }

        T val = m_deque.back();
        m_deque.pop_back();

        m_lock.unlock();

        return val;
    }

    lock_t &lockRef()
    {
        return m_lock;
    }

    bool empty()
    {
        return m_deque.empty();
    }

private:
    std::deque<T> m_deque;
    lock_t m_lock;
};
} // namespace engine

#endif