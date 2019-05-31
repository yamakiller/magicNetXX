#include "actorComponent.h"

namespace wolf
{
namespace module
{

actorComponent::actorComponent() : m_session(0), m_parent(nullptr) {}

actorComponent::~actorComponent()
{
  assert(m_suspedSession.size() == 0);
  m_proto.clear();
}

int32_t actorComponent::doInit(actor *parent, void *parm)
{
  m_parent = parent;
  doRegisterProtocol({messageId::M_ID_RESOUE, nullptr, nullptr, nullptr});
  doRegisterProtocol({messageId::M_ID_ERROR,
                      &actorComponent::staticErrorDispatch, nullptr,
                      &actorComponent::staticErrorUnPack});
  return 0;
}

int32_t actorComponent::doRun(struct message *msg)
{
  dispatchMessage(msg);
  return 1;
}

void actorComponent::doTimeout(int tm, std::function<void(void)> func)
{
  int32_t session = genSession();
  coEntry co;
  co._id = INST(actorSystem, doGenEntryId);
  co._func = func;
  if (!insertSusped(session, co))
  {
    SYSLOG_ERROR(m_parent->handle(), "Registration timer failed.");
    return;
  }
  m_parent->doTimeOut(tm, session);
}

void actorComponent::doWait(struct coEntry co)
{
  int32_t session = genSession();
  if (!suspendSleep(session, co))
  {
    return;
  }

  removeSleepSusped(co._id);
  removeSusped(session);
}

bool actorComponent::doWakeup(struct coEntry co)
{
  int32_t session = getSleepSusped(co);
  if (session == -1)
  {
    return false;
  }

  operation::worker::wakeup(co._entry);
  return true;
}

void actorComponent::quit() { m_parent->doExit(); }

int32_t actorComponent::genSession()
{
  int32_t session = ++m_session;
  if (session <= 0)
  {
    m_session = 1;
    return 1;
  }

  return session;
}

bool actorComponent::suspendSleep(int32_t session, struct coEntry co)
{
  if (!insertSusped(session, co))
  {
    return false;
  }
  insertSleepSusped(session, co);
  operation::worker::operCoYield();
  return true;
}

coEntry *actorComponent::getSusped(int32_t session)
{
  if (m_suspedSession.empty())
  {
    return nullptr;
  }

  auto it = m_suspedSession.find(session);
  if (it == m_suspedSession.end())
  {
    return nullptr;
  }

  return &it->second;
}

bool actorComponent::insertSusped(int32_t session, struct coEntry co)
{
  if (!m_suspedSession.empty())
  {
    auto it = m_suspedSession.find(session);
    if (it != m_suspedSession.end())
    {
      SYSLOG_ERROR(m_parent->handle(),
                   "Suspend failed Session repeat error({})", session);
      return false;
    }
  }

  m_suspedSession[session] = co;
  return true;
}

void actorComponent::removeSusped(int32_t session)
{
  if (m_suspedSession.empty())
  {
    return;
  }

  auto it = m_suspedSession.find(session);
  if (it == m_suspedSession.end())
  {
    return;
  }

  m_suspedSession.erase(it);
}

int32_t actorComponent::getSleepSusped(struct coEntry co)
{
  if (m_suspedSleep.empty())
  {
    return -1;
  }

  auto it = m_suspedSleep.find(co._id);
  if (it == m_suspedSleep.end())
  {
    return -1;
  }

  return m_suspedSleep[co._id];
}

void actorComponent::insertSleepSusped(int32_t session, struct coEntry co)
{
  m_suspedSleep[co._id] = session;
}

void actorComponent::removeSleepSusped(uint64_t enteryId)
{
  if (m_suspedSleep.empty())
  {
    return;
  }

  auto it = m_suspedSleep.find(enteryId);
  if (it == m_suspedSleep.end())
  {
    return;
  }

  m_suspedSleep.erase(enteryId);
}

messageProtocol *actorComponent::getProtocol(int32_t msgId)
{
  for (size_t i = 0; i < m_proto.size(); i++)
  {
    if (m_proto[i]._msgId == msgId)
    {
      return &m_proto[i];
    }
  }
  return nullptr;
}

void actorComponent::doRegisterProtocol(messageProtocol proto)
{
  messageProtocol *ptrProt = getProtocol(proto._msgId);
  if (ptrProt)
  {
    ptrProt->_msgId = proto._msgId;
    ptrProt->_dispatch = proto._dispatch;
    ptrProt->_pack = proto._pack;
    ptrProt->_unpack = proto._unpack;
    return;
  }

  m_proto.push_back(proto);
}

int32_t actorComponent::dispatchMessage(struct message *msg)
{
  int msgId = messageApi::getMessageId(msg);
  uint32_t msgSz = messageApi::getMessageSize(msg);
  void *msgData = msg->_data;
  int32_t msgSession = msg->_session;
  uint32_t msgSrc = msg->_src;
  uint32_t msgDst = msg->_dst;

  if (msgId == messageId::M_ID_RESOUE)
  {
    coEntry *ptrCo = getSusped(msgSession);
    if (ptrCo == nullptr)
    {
      unknownResponse(msgSession, msgSrc, msgData, msgSz);
      return 0;
    }
    else
    {
      util::incursivePtr<operation::task> tkPtr = ptrCo->_entry._tk.lock();
      if (!tkPtr)
      {
        unknownResponse(msgSession, msgSrc, msgData, msgSz);
        removeSusped(msgSession);
        return 0;
      }

      if (tkPtr->_state == operation::taskState::runnable ||
          tkPtr->_state == operation::taskState::done)
      {
        removeSusped(msgSession);
        return 0;
      }

      operation::worker::suspendEntry tmpEntry = std::move(ptrCo->_entry);
      removeSusped(msgSession);
      operation::worker::wakeup(tmpEntry);
      return 0;
    }
  }
  else if (msgId == messageId::M_ID_TIMEOUT)
  {
    coEntry *ptrCo = getSusped(msgSession);
    if (ptrCo == nullptr || ptrCo->_func == nullptr)
    {
      unknownResponse(msgSession, msgSrc, msgData, msgSz);
      return 0;
    }

    ptrCo->_func();
    removeSusped(msgSession);
    return 0;
  }
  else if (msgId == messageId::M_ID_QUIT)
  {
    INST(actorSystem, doUnRegister, msgDst);
    return 0;
  }
  else
  {
    messageProtocol *p = getProtocol(msgId);
    if (p == nullptr)
    {
      if (msgSession != 0)
      {
        INST(actorSystem, doSendMessage, msgSrc, msgSrc, messageId::M_ID_ERROR,
             msgSession, (void *)"");
      }
      else
      {
        unknownRequest(msgId, msgSession, msgSrc, msgData, msgSz);
      }
      return 0;
    }

    auto f = p->_dispatch;
    if (f)
    {
      f(this, msgSession, msgSrc, p->_unpack(this, msgData, msgSz));
    }
    else
    {
      if (msgSession != 0)
      {
        INST(actorSystem, doSendMessage, msgSrc, msgSrc, messageId::M_ID_ERROR,
             msgSession, (void *)"");
      }
      else
      {
        unknownRequest(msgId, msgSession, msgSrc, msgData, msgSz);
      }
    }
  }
}

void actorComponent::unknownResponse(int32_t session, uint32_t source,
                                     void *msg, uint32_t sz)
{
  SYSLOG_ERROR(m_parent->handle(), "Response message : {}",
               std::string((const char *)msg, sz).c_str());
  SYSLOG_ERROR(m_parent->handle(), "Unknown session : {} from {:08x}", session,
               source);
}

void actorComponent::unknownRequest(int32_t msgId, int32_t session,
                                    uint32_t source, void *msg, uint32_t sz)
{
  SYSLOG_ERROR(m_parent->handle(), "Unknown request ({}): {}", msgId,
               std::string((const char *)msg, sz).c_str());
  SYSLOG_ERROR(m_parent->handle(), "Unknown session : {} from {:08x}", session,
               source);
}

void actorComponent::staticErrorDispatch(void *param, int32_t session,
                                         uint32_t src, boost::any data)
{
  actorComponent *cpt = static_cast<actorComponent *>(param);
  cpt->errorDispatch(session, src, data);
}

struct errorResult
{
  const char *_message;
  uint32_t _size;
};

void actorComponent::errorDispatch(int32_t errorSession, uint32_t errorSrc,
                                   boost::any &data)
{
  struct coEntry *ptrCo = getSusped(errorSession);
  if (ptrCo == nullptr)
  {
    errorResult er = boost::any_cast<errorResult>(data);
    if (strcmp(er._message, "") != 0)
    {
      SYSLOG_ERROR(m_parent->handle(), "Source Address [:{:08x}] Error:{}",
                   errorSrc, er._message);
      return;
    }
  }
  else
  {
    uint64_t tmpId = ptrCo->_id;
    operation::worker::suspendEntry tmpEnter = std::move(ptrCo->_entry);
    removeSleepSusped(tmpId);
    removeSusped(errorSession);
    operation::worker::wakeup(tmpEnter);
  }
}

boost::any actorComponent::staticErrorUnPack(void *param, void *data,
                                             uint32_t size)
{
  errorResult er{(const char *)data, size};
  boost::any result(er);
  return result;
}

} // namespace module
} // namespace wolf