#ifndef CIS_ENGINE_DEQUE_H
#define CIS_ENGINE_DEQUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>

#include "list.h"
#include "spinlock.h"

namespace engine
{

template <typename T>
struct dequeNode2
{
  dequeNode2<T> *_prev;
  dequeNode2<T> *_next;
};

template <typename T>
class list2
{
  static_assert((std::is_base_of<dequeNode2, T>::value), "T must be baseof dequeNode2");

public:
  struct iterator
  {
    T *_ptr;
    T *_prev;
    T *_next;

    iterator() : _ptr(nullptr), _prev(nullptr), _next(nullptr) {}
    iterator(T *p) { reset(p); }
    void reset(T *p)
    {
      _ptr = p;
      _next = ptr ? (T *)_ptr->_next : nullptr;
      _prev = ptr ? (T *)_ptr->_prev : nullptr;
    }

    friend bool operator==(iterator const &lhs, iterator const &rhs)
    {
      return lhs._ptr == rhs._ptr;
    }
    friend bool operator!=(iterator const &lhs, iterator const &rhs)
    {
      return !(lhs._ptr == rhs._ptr);
    }

    iterator &operator++()
    {
      reset(_next);
      return *this;
    }
    iterator operator++(int)
    {
      iterator ret = *this;
      ++(*this);
      return ret;
    }
    iterator &operator--()
    {
      reset(_prev);
      return *this;
    }
    iterator operator--(int)
    {
      iterator ret = *this;
      --(*this);
      return ret;
    }
    T &operator*() { return *(T *)_ptr; }
    T *operator->() { return (T *)_ptr; }
  };

public:
  list2() : m_head(nullptr), m_tail(nullptr), m_count(0) {}

  list2(dequeNode2<T> *h, dequeNode2<T> *t, size_t count)
      : m_head(h), m_tail(t), m_count(count) {}

  list2(list2 const &) = delete;
  list2 &operator=(list2 const &) = delete;

  list2(list2<T> &&other)
  {
    m_head = other.m_head;
    m_tail = other.m_tail;
    m_count = other.m_count;
    other.zero();
  }

  ~list2()
  {
    assert(m_count == 0);
  }

  list2 &operator=(list2<T> &&other)
  {
    clear();
    m_head = other.m_head;
    m_tail = other.m_tail;
    m_count = other.m_count;
    other.zero();
    return *this;
  }

  iterator begin() { return iterator{m_head}; }
  iterator end() { return iterator(); }
  bool empty() const { return m_head == nullptr; }

  iterator erase(iterator it)
  {
    T *ptr = (it++)._ptr;
    erase(ptr);
    return it;
  }
  void erase(void *param)
  {
    dequeNode2<T> *ptr = dynamic_cast<dequeNode2<T> *>(param);
    assert(ptr);
    if (ptr->prev)
    {
      ptr->_prev->_next = ptr->_next;
    }
    else
    {
      m_head = m_head->next;
    }

    if (ptr->next)
    {
      ptr->_next->_prev = ptr->_prev;
    }
    else
    {
      m_tail = m_tail->_prev;
    }

    ptr->_prev = ptr->_next = nullptr;
    --m_count;

    ptr = nullptr;
  }

  void append(list2<T> &&other)
  {
    if (other.empty())
      return;

    if (empty())
    {
      *this = std::move(other);
      return;
    }

    m_tail->_next = other.m_head;
    if (other.m_head)
    {
      other.m_head->_prev = m_tail;
    }

    if (other.m_tail)
    {
      m_tail = other.m_tail;
    }
    m_count += other.m_count;
    other.zero();
  }

  list2<T> cut(size_t n)
  {
    if (empty())
    {
      return list2<T>();
    }

    if (n >= size())
    {
      list2<T> o(std::move(*this));
      return o;
    }

    if (n == 0)
    {
      return list2<T>();
    }

    list2<T> o;
    auto pos = m_head;
    for (size_t i = 1; i < n; ++i)
      pos = pos->_next;
    o.m_head = m_head;
    o.m_tail = pos;
    o.m_count = n;

    m_count -= n;
    m_head = pos->_next;

    assert(pos->_next == m_head);
    assert(m_head->_prev == this);

    pos->_next = nullptr;
    m_head->_prev = nullptr;

    return o;
  }

  size_t size() const
  {
    return m_count;
  }
  void clear()
  {
    auto it = begin();
    while (it != end())
      it = erase(it);
    zero();
  }

  void zero()
  {
    m_head = m_tail = nullptr;
    m_count = 0;
  }

  dequeNode2<T> *head() { return m_head; }
  dequeNode2<T> *tail() { return m_tail; }

public:
  dequeNode2<T> *m_head;
  dequeNode2<T> *m_tail;

  size_t m_count;
};

template <typename T, bool threadSafe>
class deque2 final
{
public:
  typedef typename std::conditional<threadSafe, spinlock, unspinlock>::type
      lock_handle;
  typedef typename std::conditional<threadSafe, std::lock_guard<spinlock>,
                                    unslk_lock_guard>::type locked_guard;

public:
  deque2()
  {
    m_head = m_tail = nullptr;
    m_count = 0;
  }

  ~deque2()
  {
    locked_guard lcked(&m_lock);
    while (m_tail)
    {
      dequeNode2<T> *prev = m_tail->prev;
      m_tail = nullptr;
      m_tail = prev;
    }
    m_head = nullptr;
  }

  void pushUnlock(void *value)
  {
    dequeNode2<T> *newNode = dynamic_cast<dequeNode2<T> *>(value);
    assert(newNode);

    if (m_head == m_tail == nullptr)
    {
      m_head = m_tail = newNode;
      newNode->_prev = newNode->_next = NULL;
    }
    else
    {
      m_tail->_next = newNode;
      newNode->_prev = m_tail;
      newNode->_next = nullptr;
      m_tail = newNode;
    }
    ++m_count;
  }

  void push(void *value)
  {
    locked_guard lcked(&m_lock);
    pushUnlock(value);
  }

  bool front(**outValue)
  {
    locked_guard lcked(&m_lock);
    if (m_head == nullptr)
    {
      *outValue = nullptr;
      return false;
    }

    *outValue = m_head;
    return true;
  }

  void *popUnlock()
  {
    if (m_head == NULL)
    {
      return nullptr;
    }

    dequeNode2<T> *tmp = nullptr;
    if (m_head == m_tail)
    {
      tmp = m_head;
      m_head = m_tail = nullptr;
    }
    else
    {
      tmp = m_head;
      m_head = m_head->_next;
      m_head->_prev = nullptr;
      tmp->_prev = tmp->_next = nullptr;
    }

    --m_count;
    return (void *)tmp;
  }

  void *pop()
  {
    locked_guard lcked(&m_lock);
    return popUnlock();
  }

  void *popBackUnlock()
  {
    if (m_tail == nullptr)
    {
      return nullptr;
    }

    dequeNode2<T> *tmp = m_tail;
    m_tail = m_tail->prev;
    m_tail->next = nullptr;
    tmp->next = tmp->prev = nullptr;

    --m_count;
    return (void *)tmp;
  }

  void *popBack()
  {
    locked_guard lcked(&m_lock);
    return popBackUnlock();
  }

  size_t size()
  {
    return m_count;
  }

  bool empty()
  {
    locked_guard lcked(&m_lock);
    if (m_head == nullptr)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  lock_handle &lockRef()
  {
    return m_lock;
  }

private:
  dequeNode2<T> *m_head;
  dequeNode2<T> *m_tail;
  lock_handle m_lock;

  volatile size_t m_count;
};

template <class T, bool threadSafe>
class deque final
{
public:
  typedef
      typename std::conditional<threadSafe, spinlock, unspinlock>::type lock_t;
  typedef typename std::conditional<threadSafe, std::lock_guard<spinlock>,
                                    unslk_lock_guard>::type locked_guard;

public:
  deque() = default;
  ~deque() = default;

  // LIFO

  void pushUnLock(const T &val) { m_deque.push_back(val); }

  void push(const T &val)
  {
    locked_guard l(m_lock);
    pushUnLock(val);
  }

  void pushUnLock(list<T> *ls)
  {

    while (!ls->empty())
    {
      m_deque.push_back(std::move(ls->pop()));
    }
    delete ls;
  }

  void push(list<T> *ls)
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

  // for steal FIFO
  list<T> *popBack(int n)
  {
    locked_guard l(m_lock);
    list<T> *result = new list<T>();

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

  list<T> *popAll()
  {
    locked_guard l(m_lock);
    return popUnLockAll();
  }

  list<T> *popUnLockAll()
  {
    list<T> *result = new list<T>();
    while (!m_deque.empty())
    {
      result->push(std::move(m_deque.front()));
      m_deque.pop_front();
    }

    return result;
  }

  lock_t &lockRef() { return m_lock; }

  size_t size()
  {
    locked_guard l(m_lock);
    return m_deque.size();
  }

  bool empty() { return m_deque.empty(); }

private:
  std::deque<T> m_deque;
  lock_t m_lock;
};
} // namespace engine

#endif