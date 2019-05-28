#ifndef CIS_ENGINE_UTIL_QUEUE_H
#define CIS_ENGINE_UTIL_QUEUE_H

#include "memory.h"
#include "spinlock.h"
#include <assert.h>
#include <mutex>

namespace wolf
{
namespace util
{
template <typename T>
class queue
{

  static constexpr auto DEFAULT_SIZE = 64;
  static constexpr auto EQ_OVERLOAD = 1024;

public:
  queue()
      : m_cap(DEFAULT_SIZE), m_head(0), m_tail(0), m_overload(0),
        m_overload_threshold(EQ_OVERLOAD)
  {
    m_arrays = (T *)memory::malloc(sizeof(T) * m_cap);
  }

  ~queue()
  {

    T val;
    while (pop(&val))
    {
      local_dropevent(&val);
    }

    memory::free(m_arrays);
    m_arrays = nullptr;
  }

  void push(T *val)
  {
    m_arrayLock.lock();
    local_push(val);
    m_arrayLock.unlock();
  }

  bool pop(T *val)
  {
    std::unique_lock<spinlock> lock(m_arrayLock);
    return local_pop(val);
  }

  spinlock *getMutexRef()
  {
    return &m_arrayLock;
  }

  int length()
  {
    int head, tail, cap;
    m_arrayLock.lock();
    head = m_head;
    tail = m_tail;
    cap = m_cap;
    m_arrayLock.unlock();

    if (head <= tail)
    {
      return tail - head;
    }
    return tail + cap - head;
  }

  int overload()
  {
    if (m_overload)
    {
      int overload = m_overload;
      m_overload = 0;
      return overload;
    }
    return 0;
  }

protected:
  virtual void local_push(T *val)
  {
    assert(val);
    m_arrays[m_tail] = *val;
    if (++m_tail >= m_cap)
    {
      m_tail = 0;
    }

    if (m_head == m_tail)
    {
      local_expand();
    }
  }

  virtual bool local_pop(T *val)
  {
    bool ret = false;
    if (m_head != m_tail)
    {
      ret = true;
      *val = m_arrays[m_head++];
      if (m_head >= m_cap)
      {
        m_head = 0;
      }

      int length = m_tail - m_head;
      if (length < 0)
      {
        length += m_cap;
      }

      while (length > m_overload_threshold)
      {
        m_overload = length;
        m_overload_threshold *= 2;
      }
    }
    else
    {
      m_overload_threshold = EQ_OVERLOAD;
    }

    return ret;
  }

  void local_expand()
  {
    T *newArrays = (T *)memory::malloc(sizeof(T) * m_cap * 2);
    for (int i = 0; i < m_cap; i++)
    {
      newArrays[i] = m_arrays[(m_head + i) % m_cap];
    }

    m_head = 0;
    m_tail = m_cap;
    m_cap *= 2;

    memory::free(m_arrays);
    m_arrays = newArrays;
  }

  virtual void local_dropevent(T *val) {}

protected:
  int m_cap;
  int m_head;
  int m_tail;
  int m_overload;
  int m_overload_threshold;

  T *m_arrays;
  spinlock m_arrayLock;
};
} // namespace util
} // namespace wolf

#endif