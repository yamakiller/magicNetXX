#ifndef WOLF_UTIL_COUNTER_H
#define WOLF_UTIL_COUNTER_H

#include "base.h"

namespace wolf
{
namespace util
{
template <typename T>
struct id_counter
{
  id_counter() { _id = ++counter(); }
  id_counter(id_counter const &) { _id = ++counter(); }
  id_counter(id_counter &&) { _id = ++counter(); }

  long getId() const { return _id; }

private:
  static atomic_t<long> &counter()
  {
    static atomic_t<long> c;
    return c;
  }

  long _id;
};
} // namespace util
} // namespace wolf

#endif