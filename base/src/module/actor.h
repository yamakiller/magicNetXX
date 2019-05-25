#ifndef CIS_ENGINE_MODULE_ACTOR_H
#define CIS_ENGINE_MODULE_ACTOR_H

#include "base.h"
#include "message.h"
#include "util/spinlock.h"
#include <deque>
#include <functional>
#include <optional>

namespace engine {
namespace module {
class actorSystem;
class actor {
  friend class actorWorker;

public:
  typedef std::function<void(message &&msg)> actorFunc;

public:
  actor(actorSystem *lsys, const char *comptName);
  virtual ~actor();

  void send(message &&msg);
  actor &operator<<(message &&m);

  std::optional<message> receive();

  inline uint32_t handle() { return m_handle; }

public:
  void dispatch();

protected:
  uint32_t m_handle;
  bool m_inglobal;
  actorFunc m_func;
  std::deque<message> m_mailbox;
  util::spinlock m_lock;

  actorSystem *m_lsys;
};

} // namespace module
} // namespace engine

#endif