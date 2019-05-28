#ifndef CIS_ENGINE_UTIL_TWOWAYLIST_H
#define CIS_ENGINE_UTIL_TWOWAYLIST_H

#include "shared_ptr.h"
#include <assert.h>

namespace wolf {
namespace util {
struct node {
  struct node *_prev;
  struct node *_next;
};

template <typename T> class list {
  static_assert((std::is_base_of<node, T>::value), "T must be baseof node");

public:
  struct iterator {
    T *ptr;
    T *prev;
    T *next;

    iterator() : ptr(nullptr), prev(nullptr), next(nullptr) {}
    iterator(T *p) { reset(p); }
    void reset(T *p) {
      ptr = p;
      next = ptr ? (T *)ptr->_next : nullptr;
      prev = ptr ? (T *)ptr->_prev : nullptr;
    }

    friend bool operator==(iterator const &lhs, iterator const &rhs) {
      return lhs.ptr == rhs.ptr;
    }
    friend bool operator!=(iterator const &lhs, iterator const &rhs) {
      return !(lhs.ptr == rhs.ptr);
    }

    iterator &operator++() {
      reset(next);
      return *this;
    }
    iterator operator++(int) {
      iterator ret = *this;
      ++(*this);
      return ret;
    }
    iterator &operator--() {
      reset(prev);
      return *this;
    }
    iterator operator--(int) {
      iterator ret = *this;
      --(*this);
      return ret;
    }
    T &operator*() { return *(T *)ptr; }
    T *operator->() { return (T *)ptr; }
  };

public:
  list() : m_head(nullptr), m_tail(nullptr), m_count(0) {}

  list(T *h, T *t, size_t count) : m_head(h), m_tail(t), m_count(count) {}

  list(list const &) = delete;
  list &operator=(list const &) = delete;

  list(list<T> &&other) {
    m_head = other.m_head;
    m_tail = other.m_tail;
    m_count = other.m_count;
    other.zero();
  }

  ~list() { assert(m_count == 0); }

  list &operator=(list<T> &&other) {
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

  iterator erase(iterator it) {
    T *ptr = (it++).ptr;
    erase(ptr);
    return it;
  }
  void erase(T *ptr) {
    if (ptr->_prev) {
      ptr->_prev->_next = ptr->_next;
    } else {
      m_head = static_cast<T *>(m_head->_next);
    }
    if (ptr->_next) {
      ptr->_next->_prev = ptr->_prev;
    } else {
      m_tail = static_cast<T *>(m_tail->_prev);
    }
    ptr->_prev = ptr->_next = nullptr;
    --m_count;
    decrementRef((shared_ref *)(ptr));
  }

  void append(list<T> &&other) {
    if (other.empty())
      return;

    if (empty()) {
      *this = std::move(other);
      return;
    }

    m_tail->_next = other.m_head;
    if (other.m_head) {
      other.m_head->_prev = m_tail;
    }

    if (other.m_tail) {
      m_tail = other.m_tail;
    }
    m_count += other.m_count;
    other.zero();
  }

  list<T> cut(size_t n) {
    if (empty()) {
      return list<T>();
    }

    if (n >= size()) {
      list<T> o(std::move(*this));
      return o;
    }

    if (n == 0) {
      return list<T>();
    }

    list<T> o;
    auto pos = m_head;
    for (size_t i = 1; i < n; ++i)
      pos = (T *)pos->_next;
    o.m_head = m_head;
    o.m_tail = pos;
    o.m_count = n;

    m_count -= n;
    m_head = (T *)pos->_next;

    assert(pos->_next == m_head);
    assert(m_head->_prev == pos);

    pos->_next = nullptr;
    m_head->_prev = nullptr;

    return o;
  }

  size_t size() const { return m_count; }
  void clear() {
    auto it = begin();
    while (it != end())
      it = erase(it);
    zero();
  }

  void zero() {
    m_head = m_tail = nullptr;
    m_count = 0;
  }

  node *head() { return (node *)m_head; }
  node *tail() { return (node *)m_tail; }

public:
  T *m_head;
  T *m_tail;

  size_t m_count;
};
} // namespace util
} // namespace wolf

#endif