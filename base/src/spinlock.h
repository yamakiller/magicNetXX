#ifndef CIS_ENGINE_SPINLOCK_H
#define CIS_ENGINE_SPINLOCK_H

#include <algorithm>
#include <atomic>

namespace engine {
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
} // namespace engine

#endif