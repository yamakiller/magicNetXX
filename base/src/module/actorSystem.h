#ifndef CIS_ENGINE_MODULE_ACTORGROUP_H
#define CIS_ENGINE_MODULE_ACTORGROUP_H

#include "actorWorker.h"
#include "util/mutex.h"
#include "util/singleton.h"
#include <memory>
#include <thread>
#include <vector>

#define ACOTR_ID_MARK 0xffffff

namespace engine {
namespace module {

class actor;
class actorSystem : public util::singleton<actorSystem> {
  friend class actorWorker;
  typedef std::shared_ptr<actor> ptrActor;
  typedef std::weak_ptr<actor> wptrActor;

public:
  actorSystem();
  ~actorSystem();

  int32_t doStart();
  void doShutdown();

  uint32_t doRegister(actor *obj);
  bool doUnRegister(uint32_t handle);

  inline void doRegiserWork(uint32_t handle) { m_workpid.doPost(handle); }

  ptrActor getGrab(uint32_t handle);

  void clear();

private:
  inline uint32_t local_addr(uint32_t handle) {
    return handle & (m_actorCap - 1);
  }

private:
  int32_t m_actorSer;
  int32_t m_actorCap;
  actorWorker m_workpid;
  std::vector<ptrActor> m_actors;
  util::wrMutex m_rwmutex;
};
} // namespace module
} // namespace engine

#endif