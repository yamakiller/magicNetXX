#include "worker.h"
#include "clock.h"
#include "scheduler.h"

namespace engine
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

int32_t worker::isBusy()
{
  if (m_ntsMark == m_nts)
  {
    return 0;
  }

  return clock::instance()->now() > m_ntsTick + 2000;
}

void worker::restBusy()
{
  m_ntsMark = m_nts;
  m_ntsTick = clock::instance()->now();
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

void worker::moveRunnable() { m_runnableQueue.push(m_newQueue.popBackAll()); }

void worker::gc() //需要修改
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
  m_ntsTick = clock::instance()->now();
  while (!m_lpsch->isShutdown())
  {
    m_runnable = m_runnableQueue.pop();
    if (m_runnable != nullptr)
    {
      ++m_nts;
      m_runnable->Run();
      if (m_gccQueue.size() > 16)
        gc();
      m_gccQueue.push(m_runnable);
      m_runnable = NULL;
      continue;
    }

    moveRunnable();
    if (!m_runnableQueue.empty())
    {
      continue;
    }

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
      fprintf(stderr, "Proc(%d).Stealed = %d", m_id, (int)steal_list_2.size());
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
      fprintf(stderr, "Proc(%d).Stealed = %d", m_id, (int)steal_list_2.size());
    }
    return steal_list_2;
  }
}

} // namespace engine