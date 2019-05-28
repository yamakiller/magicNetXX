#ifndef CIS_ENGINE_MODULE_ACTOR_H
#define CIS_ENGINE_MODULE_ACTOR_H

#include "base.h"
#include "message.h"
#include "operation/clock.h"
#include "util/queue.h"
#include <functional>

namespace engine
{
namespace module
{
class component;
class actorSystem;
class actorComponent;
class actor
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

  typedef std::function<int32_t(struct message *msg)> runFunc;

public:
  actor();
  virtual ~actor();

  uint32_t doInit(const char *name, void *parm);

  void push(struct message *msg);

  inline uint32_t handle() { return m_handle; }

protected:
  operation::clock::timeEntery doTimeOut(int time, int session);
  //void setRunFunc(runFunc func) { m_func = func; }

public:
  void dispatch();

protected:
  uint32_t m_handle;
  bool m_inglobal;
  messageQueue m_mqs;
  component *m_dll;
  actorComponent *m_cpt;
  runFunc m_func;
};

} // namespace module
} // namespace engine

#endif