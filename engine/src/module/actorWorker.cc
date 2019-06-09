#include "actorWorker.h"
#include "actor.h"
#include "actorSystem.h"
#include "config.h"
#include "operation/scheduler.h"
#include <functional>

NS_CC_M_BEGIN

actorWorker::actorWorker() {}

actorWorker::~actorWorker() {}

int32_t actorWorker::doStart() {
  return m_pid.doStart(std::bind(&actorWorker::doDispatch, this));
}

void actorWorker::doShutdown() { m_pid.doShutdown(); }

void actorWorker::doDispatch() {
  for (;;) {
    uint32_t handle = 0;
    actorSystem::ptrActor ptr = nullptr;
    if (m_works.pop(&handle)) {
      INST(
          operation::scheduler, createTask,
          [](intptr_t param) {
            uint32_t handle = param;
            actorSystem::ptrActor ptr = INST(actorSystem, getGrab, handle);
            if (ptr == nullptr) {
              SYSLOG_ERROR(handle, "Error do task fail");
              return;
            }

            ptr->dispatch();
          },
          handle, INSTGET_VAR(OPT, _stackSize));
      if (m_pid.isShutdown()) {
        break;
      }
    } else {
      break;
    }
  }
}

NS_CC_M_END