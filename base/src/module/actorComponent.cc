#include "actorComponent.h"

namespace wolf
{
namespace module
{
actorComponent::actorComponent() : m_parent(nullptr)
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

co *actorComponent::getSusped(int32_t session)
{
    if (m_susped.empty())
    {
        return nullptr;
    }

    auto it = m_susped.find(session);
    if (it == m_susped.end())
    {
        return nullptr;
    }

    return &it->second;
}

void actorComponent::removeSusped(int32_t session)
{
    if (m_susped.empty())
    {
        return;
    }

    auto it = m_susped.find(session);
    if (it == m_susped.end())
    {
        return nullptr;
    }

    m_susped.erase(it);
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
        //ptrProt->_pack = proto._pack;
        ptrProt->unpack = proto._unpack;
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

    if (msgId == messageId::M_ID_RESOUE ||
        msgId == messageId::M_ID_TIMEOUT)
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

            if (ptrCo->_func == nullptr)
                worker::wakeup(ptrCo->_entry);
            else
                worker::wakeup(ptrCo->_entry, ptrCo->_func);
            removeSusped(msgSession);
            return 0;
        }
    }
    else
    {
        messageProtocol *p = getProtocol(msgId);
        if (p == nullptr)
        {
            if (msgSession != 0)
            {
                INST(actorSystem, doSendMessage, msgSrc, msgSrc, messageId::M_ID_ERROR, msgSession, "");
            }
            else
            {
                unknownRequest(msgID, msgSession, msgSrc, msgData, msgSz);
            }
            return 0;
        }

        auto f = p->_dispatch;
        if (f)
        {
            f(msgSession, msgSrc, p->_unpack(msgData， msgSz));
        }
        else
        {
            if (msgSession != 0)
            {
                INST(actorSystem, doSendMessage, msgSrc, msgSrc, messageId::M_ID_ERROR, msgSession, "");
            }
            else
            {
                unknownRequest(msgID, msgSession, msgSrc, msgData, msgSz);
            }
        }
    }
}

void actorComponent::unknownResponse(int32_t session, uint32_t source, void *msg, uint32_t sz)
{
    SYSLOG_ERROR(m_parent->m_handle, "Response message : {}", std::string(msg, sz));
    SYSLOG_ERROR(m_parent->m_handle, "Unknown session : {} from {:08x}", session, source);
}

void actorComponent::unknownRequest(int32_t msgId, int32_t session, uint32_t source, void *msg, uint32_t sz)
{
    SYSLOG_ERROR(m_parent->m_handle, "Unknown request ({}): {}", msgId, c.tostring(msg, sz));
    SYSLOG_ERROR(m_parent->m_handle, "Unknown session : {} from {:08x}", session, source);
}

} // namespace module
} // namespace wolf