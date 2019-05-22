#ifndef CIS_ENGINE_TASK_H
#define CIS_ENGINE_TASK_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "context.h"
#include "deque.h"

namespace engine {

enum class taskState {
  runnable,
  block,
  done,
};

class worker;
typedef std::function<void()> taskFunc;

struct task : public util::node, public shared_ref {
  uint64_t _id;
  uint64_t _yieldCount;
  atomic_t<long> _supperId;

  taskFunc _func;
  context _ctx;
  worker *_lpWorker;

  taskState _state;

  task(taskFunc const &func, size_t stackSize);
  ~task();

  inline void SwapIn() { _ctx.SwapIn(); }

  inline void SwapOut() { _ctx.SwapOut(); }

private:
  void execute();
  static void defaultExecute(transfer_t trans);

private:
  task(task const &) = delete;
  task(task &&) = delete;
  task &operator=(task const &) = delete;
  task &operator=(task &&) = delete;
};
} // namespace engine

#endif