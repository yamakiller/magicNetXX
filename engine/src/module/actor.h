#ifndef WOLF_MODULE_ACTOR_H
#define WOLF_MODULE_ACTOR_H

#include "base.h"
#include "message.h"
#include "operation/clock.h"
#include "util/mobject.h"
#include "util/queue.h"

NS_CC_M_BEGIN

class component;
class actorSystem;
class actorComponent;
class actor : public util::mobject {
  friend class actorGcc;
  friend class actorWorker;
  friend class actorComponent;

  class messageQueue : public util::queue<struct message> {
  private:
    virtual void local_dropevent(struct message *val);
  };

  struct timeSingle {
    uint32_t _handle;
    int32_t _session;
  };

public:
  actor();
  virtual ~actor();

  uint32_t doInit(const char *name, void *parm);

  void doExit();

  void push(struct message *msg);

  inline uint32_t handle() { return m_handle; }

  bool isSuspedEmpty();

  actorComponent *getComponent() { return m_cpt; }

protected:
  operation::clock::timeEntery doTimeOut(int time, int session);

public:
  void dispatch();

protected:
  uint32_t m_handle;
  bool m_inglobal;
  bool m_indeath;
  uint32_t m_death_us;
  messageQueue m_mqs;
  component *m_dll;
  actorComponent *m_cpt;
};

NS_CC_M_END

#endif