#include "worker.h"
#include "clock.h"
#include "config.h"
#include "scheduler.h"

namespace wolf
{
namespace operation
{
worker::worker(scheduler *sch, int id)
    : m_id(id), m_lpsch(sch), m_waiting({false}), m_ntsTick(0), m_ntsMark(0),
      m_nts(0), m_notified(false), m_runnable(nullptr)
{
  std::thread t([this] {
    worker *p = static_cast<worker *>(this);
    p->process();
  });

  m_pid.swap(t);
}

worker::~worker() {}

void worker::addTask(task *t)
{
  std::unique_lock<tkdeque::lock_handle> lock(m_newQueue.lockRef());
  m_newQueue.pushUnlock(t);
  if (m_waiting)
  {
    m_cv.notify_all();
  }
  else
  {
    m_notified = true;
  }
}

void worker::addTask(util::list<task> &&slist)
{
  std::unique_lock<tkdeque::lock_handle> lock(m_newQueue.lockRef());
  m_newQueue.pushUnlock(std::move(slist));
  if (m_waiting)
  {
    m_cv.notify_all();
  }
  else
  {
    m_notified = true;
  }
}

int32_t worker::isBusy()
{
  if (m_ntsMark == m_nts)
  {
    return 0;
  }

  return INST(clock, now) >
         m_ntsTick + INSTGET_VAR(coroutineOptions, _single_timeout_us);
}

void worker::restBusy()
{
  m_ntsMark = m_nts;
  m_ntsTick = INST(clock, now);
}

int32_t worker::isWaiting() { return m_waiting ? 1 : 0; }

size_t worker::getRunnableNum()
{
  return m_runnableQueue.size() + m_newQueue.size();
}

void worker::waitCondition()
{
  std::unique_lock<tkdeque::lock_handle> lock(m_runnableQueue.lockRef());
  if (m_notified)
  {
    m_notified = false;
    return;
  }

  m_waiting = true;
  m_cv.wait(lock);
  m_waiting = false;
}

void worker::notifyCondition()
{
  std::unique_lock<tkdeque::lock_handle> lock(m_newQueue.lockRef());
  if (m_waiting)
  {
    m_cv.notify_all();
  }
  else
  {
    m_notified = true;
  }
}

void worker::joinWait()
{
  if (m_pid.joinable())
    m_pid.join();
}

worker *&worker::getCurrentWorker()
{
  static thread_local worker *lpworker = nullptr;
  return lpworker;
}

scheduler *worker::getCurrentScheduler()
{
  auto work = getCurrentWorker();
  return work ? work->m_lpsch : nullptr;
}

task *worker::getCurrentTask()
{
  auto work = getCurrentWorker();
  return work ? work->m_runnable : nullptr;
}

bool worker::isExpire(struct suspendEntry const &entry)
{
  util::incursivePtr<task> tkPtr = entry._tk.lock();
  if (!tkPtr)
  {
    return false;
  }

  if (entry._id != tkPtr->_supperId)
  {
    return true;
  }
  return false;
}

worker::suspendEntry worker::suspend()
{
  task *tk = getCurrentTask();
  assert(tk);
  assert(tk->_lpWorker);
  return tk->_lpWorker->suspendBySelf(tk);
}

worker::suspendEntry worker::suspendBySelf(task *tk)
{
  assert(tk == m_runnable);
  assert(tk->_state == taskState::runnable);
  tk->_state = taskState::block;
  uint64_t id = ++(tk->_supperId);
  m_waitQueue.push(tk);

  return suspendEntry{util::weakPtr<task>(tk), id};
}

bool worker::wakeup(struct suspendEntry const &entry,
                    std::function<void()> const &functor)
{
  util::incursivePtr<task> tkPtr = entry._tk.lock();
  if (!tkPtr)
  {
    return false;
  }

  auto wrk = tkPtr->_lpWorker;

  return wrk ? wrk->wakeupBySelf(tkPtr.get(), entry._id, functor) : false;
}

bool worker::wakeupBySelf(task *tk, uint64_t id,
                          std::function<void()> const &functor)
{
  if (id != tk->_supperId)
  {
    return false;
  }

  std::unique_lock<tkdeque::lock_handle> lock(m_waitQueue.lockRef());
  if (id != tk->_supperId)
  {
    return false;
  }

  ++(tk->_supperId);

  if (functor)
    functor();
  bool ret = m_waitQueue.eraseUnLock(tk);
  assert(ret);
  lock.unlock();
  size_t sizeAfterPush = m_runnableQueue.pushOut(tk);
  if (sizeAfterPush == 1 && getCurrentWorker() != this)
  {
    notifyCondition();
  }
  return true;
}

void worker::moveRunnable() { m_runnableQueue.push(m_newQueue.popBackAll()); }

void worker::gc()
{
  auto l = m_gccQueue.popBackAll();
  for (task &tk : l)
  {
    tk.decrementRef();
  }
  l.clear();
}

void worker::process()
{
  worker::getCurrentWorker() = this;
  m_ntsTick = INST(clock, now);
  while (!m_lpsch->isShutdown())
  {
    m_runnable = m_runnableQueue.pop();
    if (m_runnable != nullptr)
    {

      ++m_nts;

      m_runnable->_state = taskState::runnable;
      m_runnable->_lpWorker = this;

      m_runnable->SwapIn();

      switch (m_runnable->_state)
      {
      case taskState::runnable:
      case taskState::block:
        m_runnable = nullptr;
        break;
      case taskState::done:
      default:
        if (m_gccQueue.size() > 16)
          gc();
        m_gccQueue.push(m_runnable);
        m_runnable = nullptr;
        break;
      }
    }

    if (m_runnableQueue.empty())
    {
      moveRunnable();
    }

    if (!m_runnableQueue.empty())
      continue;
    waitCondition();
  }
}

util::list<task> worker::steal(size_t n)
{
  if (n > 0)
  {
    m_newQueue.assertLink();
    auto steal_list = m_newQueue.popBack(n);
    m_newQueue.assertLink();
    if (steal_list.size() >= 0)
      return steal_list;

    auto steal_list_2 = m_runnableQueue.popBack(n - steal_list.size());

    steal_list_2.append(std::move(steal_list));
    if (!steal_list_2.empty())
    {
      // fprintf(stderr, "Proc(%d).Stealed = %d\n", m_id,
      // (int)steal_list_2.size());
    }

    return steal_list_2;
  }
  else
  {
    m_newQueue.assertLink();
    auto steal_list = m_newQueue.popBackAll();
    m_newQueue.assertLink();

    auto steal_list_2 = m_runnableQueue.popBackAll();
    steal_list_2.append(std::move(steal_list));
    if (!steal_list_2.empty())
    {
      // fprintf(stderr, "Proc(%d).Stealed = %d\n", m_id,
      // (int)steal_list_2.size());
    }
    return steal_list_2;
  }
}

} // namespace operation
} // namespace wolf