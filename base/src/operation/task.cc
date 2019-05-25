#include "task.h"
#include "worker.h"
#include <assert.h>

namespace engine {
namespace operation {

task::task(taskFunc const &func, intptr_t funcParm, size_t stackSize)
    : _yieldCount(0), _supperId({0}),
      _ctx(&task::defaultExecute, (void *)this, stackSize), _lpWorker(nullptr),
      _func(func), _funcParm(funcParm), _state(taskState::runnable) {}

task::~task() {
  assert(!this->_prev);
  assert(!this->_next);
}

void task::execute() {
  auto fn = [this]() {
    this->_func(this->_funcParm);
    this->_func = taskFunc();
  };

  //===============================

  fn();

  //===============================
  _state = taskState::done;
  worker::operCoYield();
}

void task::defaultExecute(transfer_t trans) {
  task *tk = static_cast<task *>(trans.data);
  tk->execute();
}

} // namespace operation
} // namespace engine
