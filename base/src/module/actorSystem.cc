#include "actorSystem.h"
#include "actor.h"
#include <assert.h>

namespace wolf
{
namespace module
{

actorSystem::actorSystem() : m_actorSer(1), m_actorCap(4)
{
  m_actors.resize(m_actorCap, nullptr);
}

actorSystem::~actorSystem() { clear(); }

int32_t actorSystem::doStart(const char *componentPath)
{
  if (componentPath == nullptr)
  {
    fprintf(stderr, " Please enter the component search path!");
    exit(0);
  }
  m_cptGroup.doInit(componentPath);
  return m_workpid.doStart();
}

void actorSystem::doShutdown() { m_workpid.doShutdown(); }

uint32_t actorSystem::doRegister(actor *obj)
{
  m_rwmutex.lock_write();
  for (;;)
  {
    int i;
    for (i = 0; i < m_actorCap; i++)
    {
      uint32_t handle = (i + m_actorSer) & ACOTR_ID_MARK;
      int hash = handle & (m_actorCap - 1);
      if (m_actors[hash] == nullptr)
      {
        m_actors[hash] = ptrActor(obj);
        m_actorSer = handle + 1;
        m_rwmutex.release_write();

        return handle;
      }
    }

    assert((m_actorCap * 2 - 1) <= ACOTR_ID_MARK);
    m_actors.resize(m_actorCap * 2, nullptr);
    int nwcap = m_actorCap * 2;
    for (i = 0; i < nwcap; i++)
    {
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

bool actorSystem::doUnRegister(uint32_t handle)
{

  bool bret = false;
  m_rwmutex.lock_write();
  uint32_t hash = local_addr(handle);
  if (m_actors[hash] != nullptr && m_actors[hash]->handle() == handle)
  {
    bret = true;
    m_actors[hash] = nullptr;
  }
  m_rwmutex.release_write();
  return bret;
}

actorSystem::ptrActor actorSystem::getGrab(uint32_t handle)
{
  ptrActor result = nullptr;
  m_rwmutex.lock_read();
  uint32_t hash = local_addr(handle);
  if (m_actors[hash] && m_actors[hash]->handle() == handle)
  {
    result = m_actors[hash];
  }
  m_rwmutex.release_read();
  return result;
}

void actorSystem::clear()
{
  int i;
  for (i = 0; i < m_actorCap; i++)
  {
    m_rwmutex.lock_write();
    ptrActor ptr = m_actors[i];
    uint32_t handle = 0;
    if (ptr != nullptr)
    {
      handle = ptr->handle();
    }
    m_rwmutex.release_write();
    if (handle == 0)
      continue;
    doUnRegister(handle);
  }
}

int32_t actorSystem::doSendMessage(struct message *msg)
{
  assert(msg->_dst);
  ptrActor ptr = getGrab(msg->_dst);
  if (ptr == nullptr)
  {
    return -1;
  }

  ptr->push(msg);
  return 0;
}

int32_t actorSystem::doSendMessage(uint32_t src, uint32_t dst, int msgId, int session, void *data, size_t sz)
{
  size_t tmpsz = sz;
  if (data != nullptr && sz == 0)
  {

    if (strcmp((char *)data, "") == 0)
    {
      tmpsz = 4;
      data = util::memory::malloc(tmpsz);
      memset(data, 0, tmpsz);
    }
    else
    {
      tmpsz = strlen((char *)data);
      data = util::stringUtil::strdup(data);
    }
  }

  struct message msg = messageApi::getMessage(msgId, src, dst, session, data, tmpsz);
  return doSendMessage(&msg);
}

struct component *actorSystem::getComponent(const char *name)
{
  return m_cptGroup.getComponent(name);
}

} // namespace module
} // namespace wolf