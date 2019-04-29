#ifndef CIS_ENGINE_DEQUE_H
#define CIS_ENGINE_DEQUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>

#include "spinlock.h"

namespace engine
{
template <class T, bool threadSafe>
class deque final
{
public:
    typedef typename std::conditional<threadSafe, spinlock, unspinlock>::type lock_t;
    typedef typename std::conditional<threadSafe, std::lock_guard<spinlock>, unslk_lock_guard>::type locked_guard;

public:
    deque() = default;
    ~deque() = default;

    //LIFO

    void pushUnLock(const T &val)
    {
        m_deque.push_back(val);
    }

    void push(const T &val)
    {
        locked_guard l(m_lock);
        pushUnLock(val);
    }

    void pushUnLock(std::queue<T> *ls)
    {

        while (!ls->empty())
        {
            m_deque.push_back(std::move(ls->front()));
            ls->pop();
        }
        delete ls;
    }

    void push(std::queue<T> *ls)
    {
        locked_guard l(m_lock);
        pushUnLock(ls);
    }

    T popFront()
    {
        locked_guard l(m_lock);

        if (m_deque.empty())
        {
            return nullptr;
        }

        T val = m_deque.front();
        m_deque.pop_front();

        return val;
    }

    //for steal FIFO
    std::queue<T> *popBack(int n)
    {
        locked_guard l(m_lock);
        std::queue<T> *result = new std::queue<T>();

        if (m_deque.empty())
        {
            return result;
        }

        if (n > 0)
        {
            for (int i = 0; i < n && !m_deque.empty(); i++)
            {
                T val = m_deque.back();
                m_deque.pop_back();
                result->push(val);
            }
        }
        else
        {
            while (!m_deque.empty())
            {
                T val = m_deque.back();
                m_deque.pop_back();
                result->push(val);
            }
        }

        return result;
    }

    std::queue<T> *popAll()
    {
        locked_guard l(m_lock);
        return popUnLockAll();
    }

    std::queue<T> *popUnLockAll()
    {
        std::queue<T> *result = new std::queue<T>();
        while (!m_deque.empty())
        {
            result->push(std::move(m_deque.front()));
            m_deque.pop_front();
        }

        return result;
    }

    lock_t &lockRef()
    {
        return m_lock;
    }

    size_t size()
    {
        locked_guard l(m_lock);
        return m_deque.size();
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