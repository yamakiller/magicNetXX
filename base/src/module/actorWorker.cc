#include "actorWorker.h"
#include "actor.h"
#include "actorSystem.h"
#include "config.h"
#include "operation/scheduler.h"
#include <functional>

namespace engine
{
namespace module
{
actorWorker::actorWorker() {}

actorWorker::~actorWorker() {}

int32_t actorWorker::doStart()
{
  return m_pid.doStart(std::bind(&actorWorker::doDispatch, this));
}

void actorWorker::doShutdown() { m_pid.doShutdown(); }

void actorWorker::doDispatch()
{
  for (;;)
  {
    uint32_t handle = 0;
    actorSystem::ptrActor ptr = nullptr;
    if (m_works.pop(&handle))
    {
      INST(operation::scheduler, createTask,
           std::bind(&actorWorker::doTask, this, std::placeholders::_1), handle,
           INSTGET_VAR(coroutineOptions, _stackSize));
      if (m_pid.isShutdown())
      {
        break;
      }
    }
    else
    {
      break;
    }
  }
}

void actorWorker::doTask(intptr_t param)
{
  uint32_t handle = param;
  actorSystem::ptrActor ptr = INST(actorSystem, getGrab, handle);
  if (ptr == nullptr)
  {
    fprintf(stderr, "error do task :%u\n", handle);
    return;
  }

  ptr->dispatch();
}

} // namespace module
} // namespace engine