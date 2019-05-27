#ifndef CIS_ENGINE_MODULE_ACTOR_H
#define CIS_ENGINE_MODULE_ACTOR_H

#include "base.h"
#include "message.h"
#include "operation/clock.h"
#include "util/queue.h"
#include <functional>

namespace engine {
namespace module {
class actorSystem;
class actorComponent;
class actor {
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

  typedef std::function<int32_t(struct message *msg)> runFunc;

public:
  actor();
  virtual ~actor();

  uint32_t doInit(const char *comptName);

  void push(struct message *msg);

  inline uint32_t handle() { return m_handle; }

protected:
  virtual int32_t initialize() = 0;
  virtual void finalize(){};

protected:
  operation::clock::timeEntery doTimeOut(int time, int session);
  void setRunFunc(runFunc func) { m_func = func; }

public:
  void dispatch();

protected:
  uint32_t m_handle;
  bool m_inglobal;
  messageQueue m_mqs;
  actorComponent *m_compt;
  runFunc m_func;
};

} // namespace module
} // namespace engine

#endif