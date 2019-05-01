#ifndef CIS_ENGINE_DEQUE_H
#define CIS_ENGINE_DEQUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>

#include "list.h"
#include "spinlock.h"

namespace engine {

template <typename T> struct dequeNode2 {
  dequeNode2<T> *_prev;
  dequeNode2<T> *_next;
};

template <typename T, bool threadSafe> class deque2 final {
public:
  typedef typename std::conditional<threadSafe, spinlock, unspinlock>::type
      lock_handle;
  typedef typename std::conditional<threadSafe, std::lock_guard<spinlock>,
                                    unslk_lock_guard>::type locked_guard;

public:
  deque2() = default;
  ~deque2() = default;

  void pushUnlock(void *value) {
    dequeNode2<T> *newNode = dynamic_cast<dequeNode2<T> *>(value);
    assert(newNode);

    if (m_head == m_tail == nullptr) {
      m_head = m_tail = newNode;
      newNode->_prev = newNode->_next = NULL;
    } else {
      m_tail->_next = newNode;
      newNode->_prev = m_tail;
      newNode->_next = nullptr;
      m_tail = newNode;
    }
    ++m_count;
  }

  void push(void *value) {
    locked_guard lcked(&m_lock);
    pushUnlock(value);
  }

  void front(**outValue) {
    locked_guard lcked(&m_lock);
    if (m_head == nullptr) {
      *outValue = nullptr;
      return;
    }

    *outValue = m_head;
  }

  void *popUnlock() {
    if (m_head == NULL) {
      return nullptr;
    }

    if (m_head == m_tail) {
      dequeNode2<T> *tmp = m_head;
      m_head = m_tail = nullptr;
      return tmp;
    }

    dequeNode2<T> *tmp = m_head;
    m_head = m_head->_next;
    m_head->_prev = nullptr;
    tmp->_prev = tmp->_next = nullptr;
    return (void *)tmp;
  }

  void *pop() {
    locked_guard lcked(&m_lock);
    return popUnlock();
  }

  void *popBackUnlock() { return nullptr; }
  void *popBack() { return nullptr; }

private:
  dequeNode2<T> *m_head;
  dequeNode2<T> *m_tail;
  lock_handle m_lock;

  int32_t m_count;
};

template <class T, bool threadSafe> class deque final {
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

  void push(const T &val) {
    locked_guard l(m_lock);
    pushUnLock(val);
  }

  void pushUnLock(list<T> *ls) {

    while (!ls->empty()) {
      m_deque.push_back(std::move(ls->pop()));
    }
    delete ls;
  }

  void push(list<T> *ls) {
    locked_guard l(m_lock);
    pushUnLock(ls);
  }

  T popFront() {
    locked_guard l(m_lock);

    if (m_deque.empty()) {
      return nullptr;
    }

    T val = m_deque.front();
    m_deque.pop_front();

    return val;
  }

  // for steal FIFO
  list<T> *popBack(int n) {
    locked_guard l(m_lock);
    list<T> *result = new list<T>();

    if (m_deque.empty()) {
      return result;
    }

    if (n > 0) {
      for (int i = 0; i < n && !m_deque.empty(); i++) {
        T val = m_deque.back();
        m_deque.pop_back();
        result->push(val);
      }
    } else {
      while (!m_deque.empty()) {
        T val = m_deque.back();
        m_deque.pop_back();
        result->push(val);
      }
    }

    return result;
  }

  list<T> *popAll() {
    locked_guard l(m_lock);
    return popUnLockAll();
  }

  list<T> *popUnLockAll() {
    list<T> *result = new list<T>();
    while (!m_deque.empty()) {
      result->push(std::move(m_deque.front()));
      m_deque.pop_front();
    }

    return result;
  }

  lock_t &lockRef() { return m_lock; }

  size_t size() {
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