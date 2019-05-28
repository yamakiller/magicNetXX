#ifndef CIS_ENGINE_UTIL_REFOBJECT_H
#define CIS_ENGINE_UTIL_REFOBJECT_H

#include "base.h"
#include <string>

namespace wolf {
namespace util {
struct object_ref;
struct shared_ref;

struct releaser {
  typedef void (*func_t)(struct object_ref *ptr, void *arg);
  func_t _func;
  void *_arg;

  releaser() : _func(nullptr), _arg(nullptr) {}
  releaser(func_t func, void *arg) : _func(func), _arg(arg) {}

  inline void operator()(struct object_ref *ptr);
};

struct object_ref {

  atomic_t<long> *_reference;
  atomic_t<long> _referenceImpl;
  releaser _releaser;

  object_ref() : _referenceImpl{1} { _reference = &_referenceImpl; }
  virtual ~object_ref() {}

  bool isShared() const { return _reference != &_referenceImpl; }

  void incrementRef() { ++*_reference; }

  virtual bool decrementRef() {
    if (--*_reference == 0) {
      _releaser(this);
      return true;
    }
    return false;
  }

  long useCount() { return *_reference; }

  void setReleaser(releaser rd) { _releaser = rd; }

  object_ref(object_ref const &) = delete;
  object_ref &operator=(object_ref const &) = delete;
};

struct object_ref_impl {
  atomic_t<long> _reference;
  atomic_t<long> _weak;

  object_ref_impl() : _reference{1}, _weak{1} {}

  void incrementWeak() { ++_weak; }

  void decrementWeak() {
    if (--_weak == 0) {
      delete this;
    }
  }

  bool lock() {
    long count = _reference.load(std::memory_order_relaxed);
    do {
      if (count == 0)
        return false;
    } while (!_reference.compare_exchange_weak(count, count + 1,
                                               std::memory_order_acq_rel,
                                               std::memory_order_relaxed));

    return true;
  }

  object_ref_impl(object_ref_impl const &) = delete;
  object_ref_impl &operator=(object_ref_impl const &) = delete;
};

struct shared_ref : public object_ref {
  object_ref_impl *_impl;

  shared_ref() : _impl(new object_ref_impl) {
    this->_reference = &_impl->_reference;
  }

  virtual bool decrementRef() {
    object_ref_impl *impl = _impl;
    if (object_ref::decrementRef()) {
      std::atomic_thread_fence(std::memory_order_acq_rel);
      impl->decrementWeak();
      return true;
    }
    return false;
  }
};

inline void releaser::operator()(struct object_ref *ptr) {
  if (_func)
    _func(ptr, _arg);
  else
    delete ptr;
}

template <typename T> class incursivePtr {
public:
  incursivePtr() : _ptr(nullptr) {}
  explicit incursivePtr(T *ptr) : _ptr(ptr) {
    if (_ptr) {
      _ptr->incrementRef();
    }
  }
  ~incursivePtr() {
    if (_ptr) {
      _ptr->decrementRef();
    }
  }

  incursivePtr(incursivePtr const &other) : _ptr(other._ptr) {
    if (_ptr) {
      _ptr->incrementRef();
    }
  }
  incursivePtr(incursivePtr &&other) {
    _ptr = other._ptr;
    other._ptr = nullptr;
  }
  incursivePtr &operator=(incursivePtr const &other) {
    if (this == &other) {
      return *this;
    }
    reset();
    _ptr = other._ptr;
    if (_ptr) {
      _ptr->incrementRef();
    }
    return *this;
  }
  incursivePtr &operator=(incursivePtr &&other) {
    if (this == &other) {
      return *this;
    }
    reset();
    std::swap(_ptr, other._ptr);
    return *this;
  }

  T &operator*() const { return *_ptr; }
  T *operator->() const { return _ptr; }
  explicit operator bool() const { return !!_ptr; }
  T *get() const { return _ptr; }

  void reset() {
    if (_ptr) {
      _ptr->decrementRef();
      _ptr = nullptr;
    }
  }

  long useCount() { return _ptr ? (long)*_ptr->_reference : 0; }

  bool unique() { return useCount() == 1; }

  void swap(incursivePtr &other) { std::swap(_ptr, other._ptr); }

  friend inline bool operator==(incursivePtr const &lhs,
                                incursivePtr const &rhs) {
    return lhs._ptr == rhs._ptr;
  }
  friend inline bool operator!=(incursivePtr const &lhs,
                                incursivePtr const &rhs) {
    return lhs._ptr != rhs._ptr;
  }
  friend inline bool operator<(incursivePtr const &lhs,
                               incursivePtr const &rhs) {
    return lhs._ptr < rhs._ptr;
  }

private:
  T *_ptr;
};

template <typename T> class weakPtr {
public:
  weakPtr() : _impl(nullptr), _ptr(nullptr) {}

  explicit weakPtr(T *ptr) : _impl(nullptr), _ptr(nullptr) { reset(ptr); }

  explicit weakPtr(incursivePtr<T> const &iptr)
      : _impl(nullptr), _ptr(nullptr) {
    T *ptr = iptr.get();
    reset(ptr);
  }

  weakPtr(weakPtr const &other) : _impl(other._impl), _ptr(other._ptr) {
    if (_impl) {
      _impl->incrementWeak();
    }
  }

  weakPtr(weakPtr &&other) : _impl(nullptr), _ptr(nullptr) { swap(other); }

  weakPtr &operator=(weakPtr &&other) {
    swap(other);
    return *this;
  }

  weakPtr &operator=(weakPtr const &other) {
    if (this == &other) {
      return *this;
    }
    reset();
    if (other._impl) {
      _impl = other._impl;
      _ptr = other._ptr;
      _impl->incrementWeak();
    }
    return *this;
  }
  ~weakPtr() { reset(); }
  void swap(weakPtr &other) {
    std::swap(_impl, other._impl);
    std::swap(_ptr, other._ptr);
  }
  void reset() {
    if (_impl) {
      _impl->decrementWeak();
      _impl = nullptr;
      _ptr = nullptr;
    }
  }

  void reset(T *ptr) {
    if (_impl) {
      _impl->decrementWeak();
      _impl = nullptr;
      _ptr = nullptr;
    }

    if (!ptr) {
      return;
    }

    if (!ptr->isShared()) {
      return;
    }

    _impl = ((shared_ref *)ptr)->_impl;
    _ptr = ptr;
    _impl->incrementWeak();
  }

  incursivePtr<T> lock() const {
    if (!_impl) {
      return incursivePtr<T>();
    }

    if (!_impl->lock()) {
      return incursivePtr<T>();
    }

    incursivePtr<T> iptr(_ptr);
    _ptr->decrementRef();
    return iptr;
  }

  explicit operator bool() const { return !!_impl; }

  friend bool operator==(weakPtr<T> const &lhs, weakPtr<T> const &rhs) {
    return lhs._impl == rhs._impl && lhs._ptr == rhs._ptr;
  }

  friend bool operator<(weakPtr<T> const &lhs, weakPtr<T> const &rhs) {
    return lhs._impl < rhs._impl;
  }

  long useCount() { return _impl ? (long)_impl->_reference : 0; }

private:
  object_ref_impl *_impl;
  T *_ptr;
};

template <typename T>
typename std::enable_if<std::is_base_of<object_ref, T>::value,
                        std::shared_ptr<T>>::type
sharedFromThis(T *ptr) {
  ptr->incrementRef();
  return std::shared_ptr<T>(ptr, [](T *self) { self->decrementRef(); });
}

template <typename T, typename... Args>
typename std::enable_if<std::is_base_of<object_ref, T>::value,
                        std::shared_ptr<T>>::type
makeShared(Args &&... args) {
  T *ptr = new T(std::forward<Args>(args)...);
  return std::shared_ptr<T>(ptr, [](T *self) { self->decrementRef(); });
}

template <typename T>
typename std::enable_if<std::is_base_of<object_ref, T>::value>::type
incrementRef(T *ptr) {
  ptr->incrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<object_ref, T>::value>::type
incrementRef(T *ptr) {}
template <typename T>
typename std::enable_if<std::is_base_of<object_ref, T>::value>::type
decrementRef(T *ptr) {
  ptr->decrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<object_ref, T>::value>::type
decrementRef(T *ptr) {}
} // namespace util
} // namespace wolf

#endif