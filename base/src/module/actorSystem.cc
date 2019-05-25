#include "actorSystem.h"
#include "actor.h"
#include <assert.h>

namespace engine {
namespace module {

actorSystem::actorSystem() : m_actorSer(1), m_actorCap(4) {
  m_actors.resize(m_actorCap, nullptr);
}

actorSystem::~actorSystem() { clear(); }

int32_t actorSystem::doStart() { return m_workpid.doStart(); }

void actorSystem::doShutdown() { m_workpid.doShutdown(); }

uint32_t actorSystem::doRegister(actor *obj) {
  m_rwmutex.lock_write();
  for (;;) {
    int i;
    for (i = 0; i < m_actorCap; i++) {
      uint32_t handle = (i + m_actorCap) & ACOTR_ID_MARK;
      int hash = handle & (m_actorCap - 1);
      if (m_actors[hash] == nullptr) {
        m_actors[hash] = ptrActor(obj);
        m_actorSer = handle + 1;
        m_rwmutex.release_write();

        return handle;
      }
    }

    assert((m_actorCap * 2 - 1) <= ACOTR_ID_MARK);
    m_actors.resize(m_actorCap * 2, nullptr);
    int nwcap = m_actorCap * 2;
    for (i = 0; i < nwcap; i++) {
      if (m_actors[i] == nullptr)
        continue;
      int nwpos = m_actors[i]->handle() & (nwcap - 1);
      if (nwpos == i)
        continue;
      m_actors[nwpos] = m_actors[i];
      m_actorCap = nwcap;
    }
  }
}

bool actorSystem::doUnRegister(uint32_t handle) {

  bool bret = false;
  m_rwmutex.lock_write();
  uint32_t hash = local_addr(handle);
  if (m_actors[hash] != nullptr && m_actors[hash]->handle() == handle) {
    bret = true;
    m_actors[hash] = nullptr;
  }
  m_rwmutex.release_write();
  return bret;
}

actorSystem::ptrActor actorSystem::getGrab(uint32_t handle) {
  ptrActor result = nullptr;
  m_rwmutex.lock_read();
  uint32_t hash = local_addr(handle);
  if (m_actors[hash] && m_actors[hash]->handle() == handle) {
    result = m_actors[hash];
  }
  m_rwmutex.release_read();
  return result;
}

void actorSystem::clear() {

  int i;
  for (i = 0; i < m_actorCap; i++) {
    m_rwmutex.lock_write();
    ptrActor ptr = m_actors[i];
    uint32_t handle = 0;
    if (ptr != nullptr) {
      handle = ptr->handle();
    }
    m_rwmutex.release_write();
    if (handle == 0)
      continue;
    doUnRegister(handle);
  }
}

} // namespace module
} // namespace engine