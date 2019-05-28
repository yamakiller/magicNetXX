#ifndef WOLF_SCHEDULER_H
#define WOLF_SCHEDULER_H

#include <mutex>
#include <stdint.h>
#include <thread>
#include <vector>

#include <string>

#include "util/noncopyable.h"
#include "util/singleton.h"
#include "worker.h"

namespace wolf
{
namespace operation
{

class scheduler : public util::singleton<scheduler>, public util::noncopyable
{
  friend class worker;

public:
  static const int g_ulimitedMaxThreadNumber = 40960;

  int32_t doStart(int32_t threadNumber);

  void doShutdown();

  inline int32_t isShutdown() { return m_shutdown; }

  void createTask(taskFunc const &fn, intptr_t fnParm, size_t const &stackSize);

  scheduler();
  ~scheduler();

  std::string debug();

protected:
  scheduler(scheduler const &) = delete;
  scheduler(scheduler &&) = delete;
  scheduler &operator=(scheduler const &) = delete;
  scheduler &operator=(scheduler &&) = delete;

  void addTask(task *t);

  static void releaseTask(util::object_ref *t, void *arg);

private:
  void newWorker();

  void dispatcherWork();

  void timeTick();

private:
  std::vector<worker *> m_works;
  util::spinlock m_started;

  atomic_t<uint32_t> m_taskCount;

  int32_t m_maxThreadNumber;

  std::thread m_dispatchThread; //调度线程
  std::thread m_timeThread;

  std::mutex m_shutdownMtx;
  int32_t m_shutdown;
};

} // namespace operation
} // namespace wolf

#endif