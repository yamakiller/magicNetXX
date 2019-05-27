#include "scheduler.h"
#include "base.h"
#include "clock.h"
#include "config.h"
#include <assert.h>
#include <map>

namespace engine
{
namespace operation
{
inline atomic_t<unsigned long long> &getIdFactory()
{
  static atomic_t<unsigned long long> idFactory;
  return idFactory;
}

scheduler::scheduler()
    : m_taskCount({0}), m_maxThreadNumber(1), m_shutdown(0) {}

scheduler::~scheduler() {}

int32_t scheduler::doStart(int32_t threadNumber)
{
  if (!m_started.try_lock())
  {
    throw std::logic_error(
        "Coroutine Scheduler repeated call scheduler::doStart");
  }

  if (threadNumber < 1)
    threadNumber = std::thread::hardware_concurrency();

  m_maxThreadNumber = threadNumber;

  for (int i = 0; i < m_maxThreadNumber; i++)
  {
    newWorker();
  }

  if (m_maxThreadNumber > 1)
  {
    std::thread t([this] { this->dispatcherWork(); });
    m_dispatchThread.swap(t);
    SYSLOG_INFO(0, "Coroutine Scheduler Thread Started");
  }
  else
  {
    SYSLOG_INFO(0, "Coroutine No Scheduler Thread");
  }

  {
    std::thread t([this] { this->timeTick(); });
    m_timeThread.swap(t);
  }
  m_started.unlock();
  SYSLOG_INFO(0, "Coroutine Scheduler Max Thread[{}]", m_maxThreadNumber);

  return 0;
}

void scheduler::doShutdown()
{
  std::unique_lock<std::mutex> lock(m_shutdownMtx);
  m_shutdown = 1;
  size_t n = m_works.size();
  for (size_t i = 0; i < n; ++i)
  {
    auto p = m_works[i];
    if (p)
      p->notifyCondition();
  }

  if (m_dispatchThread.joinable())
    m_dispatchThread.join();
  if (m_timeThread.joinable())
    m_timeThread.join();
}

void scheduler::createTask(taskFunc const &fn, intptr_t fnParm,
                           size_t const &stackSize)
{
  task *t = new task(fn, fnParm, stackSize);
  t->setReleaser(util::releaser(&scheduler::releaseTask, this));
  t->_id = ++getIdFactory();
  ++m_taskCount;
  addTask(t);
}

void scheduler::releaseTask(util::object_ref *t, void *arg)
{
  scheduler *sch = static_cast<scheduler *>(arg);
  delete t;
  --sch->m_taskCount;
}

void scheduler::addTask(task *t)
{
  auto work = worker::getCurrentWorker();
  if (work && !work->isBusy())
  {
    work->addTask(t);
    return;
  }

  std::size_t pcount = m_works.size();
  std::size_t idx = rand() % pcount;

  m_works[idx]->addTask(t);
}

void scheduler::newWorker()
{
  worker *pwk = new worker(this, m_works.size());
  assert(pwk);
  m_works.push_back(pwk);
}

void scheduler::dispatcherWork()
{
  while (!m_shutdown)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(
        INSTGET_VAR(coroutineOptions, _dispatcher_thread_interval_us)));

    size_t workCount = m_works.size();
    size_t totalLoadaverage = 0;
    typedef std::multimap<size_t, size_t> LoadMap;
    LoadMap loadMaps;
    // 1.统计繁忙中的线程
    std::map<size_t, size_t> busyings;
    for (size_t i = 0; i < workCount; i++)
    {
      auto w = m_works[i];
      if (w->isBusy())
      {
        busyings[i] = w->getRunnableNum();
        w->restBusy();
      }
    }

    // 2. 计算负载情况
    for (size_t i = 0; i < workCount; i++)
    {
      auto w = m_works[i];
      size_t loadaverage = w->getRunnableNum();
      totalLoadaverage += loadaverage;

      loadMaps.insert(LoadMap::value_type{loadaverage, i});

      if (loadaverage > 0 && w->isWaiting())
      {
        w->notifyCondition();
      }
    }

    // 3. 东西
    // 阻塞线程的任务steal出来
    {
      util::list<task> tasks;
      for (auto &kv : busyings)
      {
        auto p = m_works[kv.first];
        tasks.append(p->steal(0));
      }

      if (!tasks.empty())
      {
        auto range = loadMaps.equal_range(loadMaps.begin()->first);
        size_t avg = tasks.size() / std::distance(range.first, range.second);
        if (avg == 0)
          avg = 1;

        LoadMap newLoadMaps;
        for (auto it = range.second; it != loadMaps.end(); ++it)
        {
          newLoadMaps.insert(*it);
        }

        for (auto it = range.first; it != range.second; ++it)
        {
          util::list<task> in = tasks.cut(avg);
          if (in.empty())
          {
            break;
          }

          auto w = m_works[it->second];
          w->addTask(std::move(in));
          newLoadMaps.insert(
              LoadMap::value_type{w->getRunnableNum(), it->second});
        }

        if (!tasks.empty())
          m_works[range.first->second]->addTask(std::move(tasks));

        for (auto it = range.first; it != range.second; ++it)
        {
          auto w = m_works[it->second];
          newLoadMaps.insert(
              LoadMap::value_type{w->getRunnableNum(), it->second});
        }
        newLoadMaps.swap(loadMaps);
      }
    }

    if (loadMaps.begin()->first == 0)
    {
      auto range = loadMaps.equal_range(loadMaps.begin()->first);
      size_t waitN = std::distance(range.first, range.second);
      if (waitN == loadMaps.size())
      {
        continue;
      }

      auto maxP = m_works[loadMaps.rbegin()->second];
      std::size_t stealN = (std::min)(maxP->getRunnableNum() / 2, waitN * 1024);
      auto tasks = maxP->steal(stealN);
      if (tasks.empty())
      {
        continue;
      }

      size_t avg = tasks.size() / waitN;
      if (avg == 0)
        avg = 1;

      for (auto it = range.first; it != range.second; ++it)
      {
        util::list<task> in = tasks.cut(avg);
        if (in.empty())
        {
          break;
        }

        auto w = m_works[it->second];
        w->addTask(std::move(in));
      }

      if (!tasks.empty())
      {
        m_works[range.first->second]->addTask(std::move(tasks));
      }
    }
  }

  size_t n = m_works.size();
  for (size_t i = 0; i < n; ++i)
  {
    auto w = m_works[i];
    if (w)
      w->joinWait();
  }
}

void scheduler::timeTick()
{
  while (!m_shutdown)
  {
    INST(clock, tick);
    std::this_thread::sleep_for(std::chrono::microseconds(2500));
  }
}

std::string scheduler::debug()
{
  char tmp[1024];
  std::string result = "";
  size_t n = m_works.size();
  for (size_t i = 0; i < n; ++i)
  {

    auto w = m_works[i];
    // sprintf(tmp, "线程ID:%d, 线程任务量:%llu, 线程状态:%s\n", i + 1,
    // w->m_nts,
    //        w->isWaiting() ? "阻塞" : "工作中");
    result += tmp;
  }

  sprintf(tmp, "剩余任务量:%d\n", (uint32_t)m_taskCount);
  result += tmp;
  return result;
}

} // namespace operation
} // namespace engine