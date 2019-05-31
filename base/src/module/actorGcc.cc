#include "actorGcc.h"
#include "actor.h"
#include "actorSystem.h"
#include "base.h"
#include "config.h"
#include "message.h"

namespace wolf {
namespace module {
actorGcc::actorGcc() { m_shutdown = false; }

int32_t actorGcc::doStart() {
  std::thread t([this]() { this->doLoop(); });
  m_pid.swap(t);
  return 0;
}

void actorGcc::doShutdown() {
  m_shutdown = true;
  if (m_pid.joinable()) {
    m_pid.join();
  }
}

void actorGcc::enterGcc(uint32_t actorId) { m_qGcc.push(&actorId); }

void actorGcc::doLoop() {
  for (;;) {
    if (m_shutdown && m_qGcc.length() == 0) {
      break;
    }
    std::this_thread::sleep_for(
        std::chrono::microseconds(INSTGET_VAR(OPT, _actor_gcc_sleep_us)));
    int num = m_qGcc.length();
    for (int i = 0; i < m_qGcc.length(); i++) {
      uint32_t handle;
      m_qGcc.pop(&handle);
      actorSystem::ptrActor ptr = INST(actorSystem, getGrab, handle);
      if (ptr == nullptr) {
        continue;
      }

      if (!ptr->m_indeath) {
        continue;
      }

      ptr->m_death_us += INSTGET_VAR(OPT, _actor_gcc_sleep_us);
      if (ptr->isSuspedEmpty() &&
          ptr->m_death_us > INSTGET_VAR(OPT, _actor_gcc_timeout_us)) {
        INST(actorSystem, doSendMessage, ptr->m_handle, ptr->m_handle,
             messageId::M_ID_QUIT, 0, (void *)"");
        continue;
      }

      m_qGcc.push(&handle);
    }
  }
}

} // namespace module
} // namespace wolf