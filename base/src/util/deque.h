#ifndef CIS_ENGINE_UTIL_TWOWAYDEQUE_H
#define CIS_ENGINE_UTIL_TWOWAYDEQUE_H

#include "base.h"
#include "list.h"
#include "spinlock.h"

namespace wolf {

namespace util {
template <typename T, bool threadSafe> class deque final {
public:
  typedef typename std::conditional<threadSafe, spinlock, unspinlock>::type
      lock_handle;
  typedef typename std::conditional<threadSafe, std::lock_guard<spinlock>,
                                    unslk_lock_guard>::type locked_guard_hd;

public:
  deque() {
    m_head = m_tail = nullptr;
    m_count = 0;
  }

  ~deque() {
    locked_guard_hd lcked(m_lock);
    while (m_tail) {
      node *prev = m_tail->_prev;
      decrementRef(static_cast<T *>(m_tail));
      m_tail = prev;
    }
    m_head = nullptr;
  }

  void pushUnlock(T *value) {
    node *newNode = static_cast<node *>(value);

    if (m_head == nullptr) {
      m_head = m_tail = newNode;
      newNode->_prev = newNode->_next = NULL;
    } else {
      m_tail->_next = newNode;
      newNode->_prev = m_tail;
      newNode->_next = nullptr;
      m_tail = newNode;
    }

    incrementRef(value);
    ++m_count;
  }

  size_t pushOutUnlock(T *value) {
    pushUnlock(value);
    return m_count;
  }

  void pushUnlock(list<T> &&s) {
    if (s.empty()) {
      return;
    }

    if (m_head == nullptr) {
      m_head = s.head();
      m_tail = s.tail();
      m_count = s.size();
      s.zero();
      return;
    }

    m_tail->_next = s.head();
    s.head()->_prev = m_tail;
    m_tail = s.tail();
    m_count += s.size();
    s.zero();
  }

  void push(T *value) {
    locked_guard_hd lcked(m_lock);
    pushUnlock(value);
  }

  size_t pushOut(T *value) {
    locked_guard_hd lcked(m_lock);
    return pushOutUnlock(value);
  }

  void push(list<T> &&s) {
    locked_guard_hd lcked(m_lock);
    pushUnlock(std::move(s));
  }

  bool front(T **outValue) {
    locked_guard_hd lcked(m_lock);
    if (m_head == nullptr) {
      *outValue = nullptr;
      return false;
    }

    *outValue = (T *)m_head;
    return true;
  }

  T *popUnlock() {
    if (m_head == NULL) {
      return nullptr;
    }

    node *tmp = nullptr;
    if (m_head == m_tail) {
      tmp = m_head;
      m_head = m_tail = nullptr;
    } else {
      tmp = m_head;
      m_head = m_head->_next;
      m_head->_prev = nullptr;
      tmp->_prev = tmp->_next = nullptr;
    }

    decrementRef((T *)tmp);
    --m_count;
    return (T *)tmp;
  }

  T *pop() {
    locked_guard_hd lcked(m_lock);
    return popUnlock();
  }

  T *popBackUnlock() {
    if (m_tail == nullptr) {
      return nullptr;
    }

    node *tmp = m_tail;
    m_tail = m_tail->_prev;
    m_tail->_next = nullptr;
    tmp->_next = tmp->_prev = nullptr;

    --m_count;
    return (T *)tmp;
  }

  list<T> popBackAllUnlock() {
    if (m_head == nullptr) {
      return list<T>();
    }

    T *first = (T *)m_head;
    T *last = (T *)m_tail;
    size_t c = m_count;

    m_count = 0;
    m_head = m_tail = nullptr;

    return list<T>(first, last, c);
  }

  list<T> popBackUnlock(uint32_t n) {
    if (m_head == m_tail) {
      return list<T>();
    }

    node *last = m_tail;
    node *first = last;
    uint32_t c = 1;

    for (; c < n && first != m_head; ++c) {
      assert(first);
      first = first->_prev;
    }

    if (first == m_head) {
      m_head = m_tail = nullptr;
      m_count = 0;
    } else {
      m_tail = first->_prev;
      first->_prev = m_tail->_next = nullptr;
      m_count -= c;
    }

    return list<T>((T *)first, (T *)last, c);
  }

  bool eraseUnLock(T *hook) {

    assert(m_head != nullptr);

    if (hook->_prev) {
      hook->_prev->_next = hook->_next;
    }

    if (hook->_next) {
      hook->_next->_prev = hook->_prev;
    } else if (hook == m_tail) {
      m_tail = m_tail->_prev;
    }

    if (m_tail == nullptr || m_head == nullptr) {
      m_tail = m_head = nullptr;
    }
    hook->_next = hook->_prev = nullptr;
    assert(m_count > 0);
    --m_count;
    decrementRef((T *)hook);
    return true;
  }

  T *popBack() {
    locked_guard_hd lcked(m_lock);
    return popBackUnlock();
  }

  list<T> popBackAll() {
    locked_guard_hd lcked(m_lock);
    return popBackAllUnlock();
  }

  list<T> popBack(uint32_t n) {
    locked_guard_hd lcked(m_lock);
    return popBackUnlock(n);
  }

  bool erase(T *hook) {
    locked_guard_hd lcked(m_lock);
    return eraseUnLock(hook);
  }

  size_t size() { return m_count; }

  bool empty() {
    locked_guard_hd lcked(m_lock);
    if (m_head == nullptr) {
      return true;
    } else {
      return false;
    }
  }

  lock_handle &lockRef() { return m_lock; }

  void assertLink() {
    locked_guard_hd lcked(m_lock);
    if (m_head == nullptr) {
      return;
    }

    assert(!m_head->_prev);
    assert(m_tail);
    assert(!m_tail->_next);

    int n = 0;
    node *pos = m_tail;
    for (; pos != m_head; pos = pos->_prev, ++n) {
      assert(!!pos->_prev);
    }
    assert(pos == m_head);
  }

private:
  node *m_head;
  node *m_tail;
  lock_handle m_lock;

  volatile size_t m_count;
};
} // namespace util
} // namespace wolf

#endif