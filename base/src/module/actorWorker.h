#ifndef CIS_ENGINE_MODULE_ACTORWORKER_H
#define CIS_ENGINE_MODULE_ACTORWORKER_H

#include "util/post.h"
#include "util/queue.h"
#include <condition_variable>
#include <mutex>

namespace wolf
{
namespace module
{

class actorWorker
{
public:
  actorWorker();
  ~actorWorker();

  void doPost(uint32_t handle)
  {
    m_works.push(&handle);
    m_pid.doPost();
  }

  int32_t doStart();
  void doShutdown();

private:
  void doDispatch();

private:
  util::queue<uint32_t> m_works;
  util::post m_pid;
};
} // namespace module
} // namespace wolf

#endif