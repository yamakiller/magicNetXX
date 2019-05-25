#ifndef CIS_ENGINE_UTIL_SPINLOCK_H
#define CIS_ENGINE_UTIL_SPINLOCK_H

#include <algorithm>
#include <atomic>

namespace engine {
namespace util {
struct spinlock {
  std::atomic_flag flag;

  spinlock() : flag{false} {}

  inline void lock() {
    while (flag.test_and_set(std::memory_order_acquire))
      ;
  }

  inline bool try_lock() {
    return !flag.test_and_set(std::memory_order_acquire);
  }

  inline void unlock() { flag.clear(std::memory_order_release); }
};

struct unspinlock {
  void lock() {}
  bool is_lock() { return false; }
  bool try_lock() { return true; }
  void unlock() {}
};

struct unslk_lock_guard {
  template <typename Mutex> explicit unslk_lock_guard(Mutex &) {}
};
} // namespace util
} // namespace engine

#endif