#include "actor.h"
#include "actorSystem.h"

namespace engine {
namespace module {

actor::actor(actorSystem *lsys, const char *comptName)
    : m_inglobal(false), m_lsys(lsys) {
  m_handle = m_lsys->doRegister(this);
  //载入so文件
}

actor::~actor() { m_lsys = nullptr; }

void actor::send(message &&msg) {
  std::unique_lock<util::spinlock> lock(m_lock);
  m_mailbox.emplace_back(std::move(msg));
  if (!m_inglobal) {
    m_lsys->doRegiserWork(m_handle);
    m_inglobal = true;
  }
}

actor &actor::operator<<(message &&m) {
  send(std::move(m));
  return *this;
}

std::optional<message> actor::receive() {
  std::unique_lock<util::spinlock> lock(m_lock);
  if (!m_mailbox.empty()) {
    message m = std::move(m_mailbox.front());
    m_mailbox.pop_front();
    return std::move(m);
  }

  m_inglobal = false;
  return std::nullopt;
}

void actor::dispatch() {
  for (;;) {
    if (std::optional<message> m = receive()) {
      if (m_func == nullptr) {
        // TODO: 释放内存 message msg = std::move(*m)
        continue;
      }

      m_func(std::move(*m));

      // TODO: 是否需要释放内存
    } else {
      break;
    }
  }
}

} // namespace module
} // namespace engine