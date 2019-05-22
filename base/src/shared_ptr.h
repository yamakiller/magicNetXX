#ifndef CIS_ENGINE_REFOBJECT_H
#define CIS_ENGINE_REFOBJECT_H

#include "base.h"
#include <string>

namespace engine {

struct shared_ref;

struct releaser {
  typedef void (*func_t)(struct shared_ref *ptr, void *arg);
  func_t _func;
  void *_arg;

  releaser() : _func(nullptr), _arg(nullptr) {}
  releaser(func_t func, void *arg) : _func(func), _arg(arg) {}

  inline void operator()(struct shared_ref *ptr);
};

struct shared_ref {
  atomic_t<long> _reference;
  releaser _releaser;

  shared_ref() : _reference{1} {}
  virtual ~shared_ref() {}

  void incrementRef() { ++_reference; }

  bool decrementRef() {
    if (--_reference == 0) {
      _releaser(this);
      return true;
    }
    return false;
  }

  long useCount() { return _reference; }

  void setReleaser(releaser rd) { _releaser = rd; }

  shared_ref(shared_ref const &) = delete;
  shared_ref &operator=(shared_ref const &) = delete;
};

inline void releaser::operator()(struct shared_ref *ptr) {
  if (_func)
    _func(ptr, _arg);
  else
    delete ptr;
}

template <typename T>
typename std::enable_if<std::is_base_of<shared_ref, T>::value>::type
incrementRef(T *ptr) {
  ptr->incrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<shared_ref, T>::value>::type
incrementRef(T *ptr) {}
template <typename T>
typename std::enable_if<std::is_base_of<shared_ref, T>::value>::type
decrementRef(T *ptr) {
  ptr->decrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<shared_ref, T>::value>::type
decrementRef(T *ptr) {}

///////////////////////////////////////

// 创建协程的源码文件位置
/*struct SourceLocation
{
    const char *file_ = nullptr;
    int lineno_ = 0;

    void Init(const char *file, int lineno)
    {
        file_ = file, lineno_ = lineno;
    }

    friend bool operator<(SourceLocation const &lhs, SourceLocation const &rhs)
    {
        if (lhs.lineno_ != rhs.lineno_)
            return lhs.lineno_ < rhs.lineno_;

        if (lhs.file_ == rhs.file_)
            return false;

        if (lhs.file_ == nullptr)
            return true;

        if (rhs.file_ == nullptr)
            return false;

        return strcmp(lhs.file_, rhs.file_) == -1;
    }

    std::string ToString() const
    {
        std::string s("{file:");
        if (file_)
            s += file_;
        s += ", line:";
        s += std::to_string(lineno_) + "}";
        return s;
    }
};*/
} // namespace engine

#endif