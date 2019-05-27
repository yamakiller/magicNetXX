#include "actor.h"
#include "actorSystem.h"

namespace engine {
namespace module {

void actor::messageQueue::local_dropevent(struct message *val) {
  util::memory::free(val->_data);
  uint32_t source = val->_dst;
  assert(source);

  INST(actorSystem, doSendMessage, source, source, messageId::M_ID_ERROR);
}

actor::actor() : m_inglobal(false), m_compt(nullptr), m_func(nullptr) {}

actor::~actor() { finalize(); }

uint32_t actor::doInit(const char *comptName) {
  m_handle = INST(actorSystem, doRegister, this);

  if (initialize() == 0) {
    SYSLOG_INFO(m_handle, "LAUNCH {} SUCCESS", comptName);
    return m_handle;
  } else {
    SYSLOG_INFO(m_handle, "LAUNCH {} FAIL", comptName);
    INST(actorSystem, doUnRegister, m_handle);
    m_handle = 0;
    return 0;
  }
}

void actor::push(struct message *msg) {
  m_mqs.push(msg);
  m_mqs.getMutexRef()->lock();
  if (!m_inglobal) {
    INST(actorSystem, doRegiserWork, m_handle);
    m_inglobal = true;
  }
  m_mqs.getMutexRef()->unlock();
}

operation::clock::timeEntery actor::doTimeOut(int time, int session) {
  struct timeSingle *tm =
      (struct timeSingle *)util::memory::malloc(sizeof(*tm));
  assert(tm);
  tm->_handle = m_handle;
  tm->_session = session;
  auto func = [](void *parm) {
    struct timeSingle *tm = static_cast<struct timeSingle *>(parm);
    uint32_t handle = tm->_handle;
    int32_t session = tm->_session;
    util::memory::free(parm);
    struct message msg;
    INST(actorSystem, doSendMessage, 0, handle, messageId::M_ID_TIMEOUT,
         session);
  };
  return INST(operation::clock, timeOut, time, func, (const void *)tm);
}

void actor::dispatch() {
  struct message msg;
  for (;;) {
    if (m_mqs.pop(&msg)) {
      if (m_func == nullptr) {
        util::memory::free(msg._data);
        continue;
      }

      int ref = m_func(&msg);
      if (ref) {
        util::memory::free(msg._data);
      }

      INST(actorSystem, doRegiserWork, m_handle);

      break;
    } else {
      break;
    }
  }
}

} // namespace module
} // namespace engine