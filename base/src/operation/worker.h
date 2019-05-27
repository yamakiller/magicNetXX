// WorkSteal

#ifndef CIS_ENGINE_WORKER_H
#define CIS_ENGINE_WORKER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "task.h"
#include "util/deque.h"

namespace engine
{
namespace operation
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

  static scheduler *getCurrentScheduler();

  static task *getCurrentTask();

  inline static void operCoYield();

  size_t getRunnableNum();

  inline scheduler *getScheduler() { return m_lpsch; }

  struct suspendEntry
  {
    util::weakPtr<task> _tk;
    uint64_t _id;

    explicit operator bool() const { return !!_tk; }

    friend bool operator==(suspendEntry const &lhs, suspendEntry const &rhs)
    {
      return lhs._tk == rhs._tk && lhs._id == rhs._id;
    }

    friend bool operator<(suspendEntry const &lhs, suspendEntry const &rhs)
    {
      if (lhs._id == rhs._id)
        return lhs._tk < rhs._tk;
      return lhs._id < rhs._id;
    }

    bool isExpire() const { return worker::isExpire(*this); }
  };

  static struct suspendEntry suspend();

  static bool wakeup(struct suspendEntry const &entry,
                     std::function<void()> const &functor = NULL);

  static bool isExpire(struct suspendEntry const &entry);

private:
  void waitCondition();

  void notifyCondition();

  void joinWait();

  void moveRunnable();

  int32_t isBusy();

  void restBusy();

  int32_t isWaiting();

  void addTask(task *t);

  void addTask(util::list<task> &&slist);

  util::list<task> steal(size_t n);

  void gc();

  void process();

private:
  inline void coYield();

  struct suspendEntry suspendBySelf(task *tk);

  bool wakeupBySelf(task *tk, uint64_t id,
                    std::function<void()> const &functor);

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
  typedef util::deque<task, true> tkdeque;
  tkdeque m_runnableQueue;
  tkdeque m_newQueue;
  tkdeque m_waitQueue;

  typedef util::deque<task, false> untkdeque;
  untkdeque m_gccQueue; //TODO 资源的回复
};

inline void worker::operCoYield()
{
  auto work = getCurrentWorker();
  if (work)
  {
    work->coYield();
  }
}

inline void worker::coYield()
{
  task *tk = getCurrentTask();
  assert(tk);

  ++tk->_yieldCount;
  tk->SwapOut();
}
} // namespace operation
} // namespace engine

#endif
