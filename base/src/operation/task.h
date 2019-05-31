#ifndef WOLF_TASK_H
#define WOLF_TASK_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "context.h"
#include "util/deque.h"

namespace wolf {
namespace operation {
enum class taskState {
  runnable,
  block,
  done,
};

class worker;
typedef std::function<void(intptr_t)> taskFunc;

struct task : public util::node, public util::shared_ref {
  uint64_t _id;
  uint64_t _yieldCount;
  atomic_t<long> _supperId;

  taskFunc _func;
  intptr_t _funcParm;

  context _ctx;
  worker *_lpWorker;

  taskState _state;

  task(taskFunc const &func, intptr_t funcParm, size_t stackSize);
  ~task();

  inline void SwapIn() { _ctx.SwapIn(); }

  inline void SwapOut() { _ctx.SwapOut(); }

public:
  void *operator new(size_t size) { return util::memory::malloc(size); }

  void *operator new[](size_t size) { return util::memory::malloc(size); }

  void operator delete(void *p) { util::memory::free(p); }

  void operator delete[](void *p) { util::memory::free(p); }

private:
  void execute();
  static void defaultExecute(void *data);

private:
  task(task const &) = delete;
  task(task &&) = delete;
  task &operator=(task const &) = delete;
  task &operator=(task &&) = delete;
};

} // namespace operation
} // namespace wolf

#endif