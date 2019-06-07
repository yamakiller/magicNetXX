#ifndef WOLF_MODULE_ACTORGROUP_H
#define WOLF_MODULE_ACTORGROUP_H

#include "actorGcc.h"
#include "actorWorker.h"
#include "base.h"
#include "componentGroup.h"
#include "operation/worker.h"
#include "util/mutex.h"
#include "util/singleton.h"
#include <memory>
#include <thread>
#include <vector>

#define ACOTR_ID_MARK 0xffffff

NS_CC_M_BEGIN

struct coEntry {
  uint64_t _id;
  operation::worker::suspendEntry _entry;
  std::function<void(void)> _func;
};

class actor;
class actorComponent;
class actorSystem : public util::singleton<actorSystem> {
  friend class actorGcc;
  friend class actorWorker;
  typedef std::shared_ptr<actor> ptrActor;
  typedef std::weak_ptr<actor> wptrActor;

public:
  actorSystem();
  ~actorSystem();

  int32_t doStart(const char *componentPath);
  void doShutdown();

  uint32_t doRegister(actor *obj);
  bool doUnRegister(uint32_t handle);

  inline void doRegiserWork(uint32_t handle) { m_workPid.doPost(handle); }

  struct coEntry doCreateCo();

  ptrActor getGrab(uint32_t handle);

  void clear();

public:
  int32_t doSendMessage(struct message *msg);
  int32_t doSendMessage(uint32_t src, uint32_t dst, int msgId, int session = 0,
                        void *data = nullptr, size_t sz = 0);
  struct component *getComponent(const char *name);

  uint64_t doGenEntryId();

  void doEnterGcc(uint32_t actorId);

private:
  inline uint32_t local_addr(uint32_t handle) {
    return handle & (m_actorCap - 1);
  }

private:
  int32_t m_actorSer;

  int32_t m_actorCap;

  actorWorker m_workPid;

  actorGcc m_gccPid;

  std::vector<ptrActor> m_actors;

  atomic_t<uint64_t> m_entryId;

  componentGroup m_cptGroup;

  util::wrMutex m_rwmutex;
};

NS_CC_M_END

#endif