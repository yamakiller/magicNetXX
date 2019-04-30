// WorkSteal

#ifndef CIS_ENGINE_WORKER_H
#define CIS_ENGINE_WORKER_H

#include "actor.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "deque.h"
#include "task.h"

namespace engine
{

class scheduler;

class worker
{
  friend class scheduler;

public:
  worker(scheduler *sch, int id);
  ~worker();

  inline int Id() { return m_id; }

  static worker *&getCurrentWorker();

  size_t getRunnableNum();

private:
  void waitCondition();

  void notifyCondition();

  void joinWait();

  void moveRunnable();

  int32_t isBusy();

  void restBusy();

  int32_t isWaiting();

  void addTask(task *t);

  list<task *> *steal(size_t n);

  void gc();

  void process();

private:
  int m_id;
  scheduler *m_lpsch;
  std::thread m_pid;
  //条件变量
  std::condition_variable_any m_cv;
  std::atomic_bool m_waiting;
  //繁忙状态
  volatile int64_t m_ntsTick;
  volatile uint64_t m_ntsMark;
  volatile uint64_t m_nts;
  bool m_notified;
  //--------------------
  task *m_runnable;
  typedef deque<task *, true> tkdeque;
  tkdeque m_runnableQueue;
  tkdeque m_newQueue;
  tkdeque m_waitQueue;

  typedef deque<task *, false> untkdeque;
  untkdeque m_gccQueue;
};
} // namespace engine

#endif
