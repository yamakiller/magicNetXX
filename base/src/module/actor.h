#ifndef WOLF_MODULE_ACTOR_H
#define WOLF_MODULE_ACTOR_H

#include "base.h"
#include "message.h"
#include "operation/clock.h"
#include "util/queue.h"
#include "util/mobject.h"

namespace wolf
{
namespace module
{
class component;
class actorSystem;
class actorComponent;
class actor : public util::mobject
{
  friend class actorWorker;
  friend class actorComponent;

  class messageQueue : public util::queue<struct message>
  {
  private:
    virtual void local_dropevent(struct message *val);
  };

  struct timeSingle
  {
    uint32_t _handle;
    int32_t _session;
  };

public:
  actor();
  virtual ~actor();

  uint32_t doInit(const char *name, void *parm);

  void push(struct message *msg);

  inline uint32_t handle() { return m_handle; }

protected:
  operation::clock::timeEntery doTimeOut(int time, int session);

public:
  void dispatch();

protected:
  uint32_t m_handle;
  bool m_inglobal;
  messageQueue m_mqs;
  component *m_dll;
  actorComponent *m_cpt;
};

} // namespace module
} // namespace wolf

#endif