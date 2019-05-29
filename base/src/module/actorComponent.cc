#include "actor.h"
#include "actorComponent.h"

namespace wolf
{
namespace module
{
actorComponent::actorComponent() : m_session(0),
                                   m_parent(nullptr)
{
}

actorComponent::~actorComponent()
{
}

int32_t actorComponent::doInit(actor *parent, void *parm)
{
    m_parent = parent;
    return 0;
}

int32_t actorComponent::doRun(struct message *msg)
{
    int32_t succ = dispatchMessage(msg);

    return 1;
}

void actorComponent::doTimeout(int time, std::function<void(void)> func)
{
}

void actorComponent::doWait(struct co)
{
    int32_t session = genSession();
    if (!suspendSleep(session, co))
    {
        return;
    }
    removeSusped(session);
    removeSusped(co._entry._id);
}

bool actorComponent::doWakeup(struct co)
{
    int32_t session = getSleepSusped(co);
    if (session == -1)
    {
        return false;
    }

    operation::worker::wakeup(co._entery);
    return true;
}

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

bool actorComponent::suspendSleep(int32_t session, struct co)
{
    if (!insertSusped(session, co))
    {
        return false;
    }
    insertSleepSusped(session, co);
    operation::worker::operCoYield();
    return true;
}

co *actorComponent::getSusped(int32_t session)
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

bool actorComponent::insertSusped(int32_t session struct co)
{
    if (!m_suspedSession.empty())
    {
        auto it = m_suspedSession.find(session);
        if (it != m_suspedSession.end())
        {
            SYSLOG_ERROR(m_parent->handle(), "Suspend failed Session repeat error({})", session)
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

int32_t actorComponent::getSleepSusped(struct co)
{
    if (m_suspedSleep.empty())
    {
        return -1;
    }

    auto it = m_suspedSleep.find(co._entry._id);
    if (it == m_suspedSleep.end())
    {
        return -1;
    }

    return m_suspedSleep[co._entry._id];
}

void actorComponent::insertSleepSusped(int32_t session, struct co)
{
    m_suspedSleep[co._entry._id] = session;
}

void actorComponent::removeSleepSusped(struct co)
{
    if (m_suspedSleep.empty())
    {
        return;
    }

    auto it = m_suspedSleep.find(co._entry._id);
    if (it == m_suspedSleep.end())
    {
        return;
    }

    m_suspedSleep.erase(co._entry._id);
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

void actorComponent::registerProtocol(messageProtocol proto)
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
        co *ptrCo = getSusped(msgSession);
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
                return 0;
            }

            if (tkPtr->_state == operation::taskState::runnable ||
                tkPtr->_state == operation::taskState::done)
            {
                removeSusped(msgSession);
                return 0;
            }

            operation::worker::wakeup(ptrCo->_entry);
            removeSusped(msgSession);
            return 0;
        }
    }
    else if (msgId == messageId::M_ID_TIMEOUT)
    {
        co *ptrCo = getSusped(msgSession);
        if (ptrCo == nullptr || ptrCo->_func == nullptr)
        {
            unknownResponse(msgSession, msgSrc, msgData, msgSz);
            return 0;
        }

        ptrCo->_func();
        removeSusped(msgSession);
        return 0；
    }
    else
    {
        messageProtocol *p = getProtocol(msgId);
        if (p == nullptr)
        {
            if (msgSession != 0)
            {
                INST(actorSystem, doSendMessage, msgSrc, msgSrc, messageId::M_ID_ERROR, msgSession, (void *)"");
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
            f(msgSession, msgSrc, p->_unpack(msgData, msgSz));
        }
        else
        {
            if (msgSession != 0)
            {
                INST(actorSystem, doSendMessage, msgSrc, msgSrc, messageId::M_ID_ERROR, msgSession, (void *)"");
            }
            else
            {
                unknownRequest(msgId, msgSession, msgSrc, msgData, msgSz);
            }
        }
    }
}

void actorComponent::unknownResponse(int32_t session, uint32_t source, void *msg, uint32_t sz)
{
    SYSLOG_ERROR(m_parent->handle(), "Response message : {}", std::string((const char *)msg, sz).c_str());
    SYSLOG_ERROR(m_parent->handle(), "Unknown session : {} from {:08x}", session, source);
}

void actorComponent::unknownRequest(int32_t msgId, int32_t session, uint32_t source, void *msg, uint32_t sz)
{
    SYSLOG_ERROR(m_parent->handle(), "Unknown request ({}): {}", msgId, std::string((const char *)msg, sz).c_str());
    SYSLOG_ERROR(m_parent->handle(), "Unknown session : {} from {:08x}", session, source);
}

} // namespace module
} // namespace wolf