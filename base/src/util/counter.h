#ifndef CIS_ENGINE_UTIL_COUNTER_H
#define CIS_ENGINE_UTIL_COUNTER_H

#include "base.h"

namespace engine {
namespace util {
template <typename T> struct id_counter {
  id_counter() { _id = ++counter(); }
  id_counter(id_counter const &) { _id = ++counter(); }
  id_counter(id_counter &&) { _id = ++counter(); }

  long getId() const { return _id; }

private:
  static atomic_t<long> &counter() {
    static atomic_t<long> c;
    return c;
  }

  long _id;
};
} // namespace util
} // namespace engine

#endif