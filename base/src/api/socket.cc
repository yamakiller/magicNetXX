#include "socket.h"
#include "network/socketSystem.h"
#include "util/memory.h"
#include "module/message.h"

namespace wolf
{
namespace api
{
socket_maps socketApi::m_socketPool;
util::spinlock socketApi::m_socketMutex;

buffer_pools socketApi::m_bufferPool;
util::spinlock socketApi::m_bufferMutex;

void socketApi::doRequire(module::actorComponent *cpt)
{
    doRegisterProtocol({module::messageId::M_ID_SOCKET, &socketApi::staticSocketDispatch, nullptr, &socketApi::staticSocketUnPack});
}

void socketApi::staticSocketDispatch(void *param, int32_t session, uint32_t src, boost::any data)
{
    int isFree = true;
    module::actorComponent *cpt = static_cast<module::actorComponent *>(param);
    network::socketMessage *msg = boost::any_cast<network::socketMessage *>(data);
    switch (msg->_type)
    {
    case network::socketMessageType::M_SOCKET_ACCEPT:
        socketApi::onAccept(cpt->getHandle(), msg->_id, msg->_ext, msg->_buffer);
        break;
    case network::socketMessageType::M_SOCKET_DATA:
        socketApi::onData(cpt->getHandle(), msg->_id, msg->_ext, msg->_buffer);
        isFree = false;
        break;
    case network::socketMessageType::M_SOCKET_CLOSE:
        socketApi::onClose(cpt->getHandle(), msg->_id);
        break;
    case network::socketMessageType::M_SOCKET_START:
        socketApi::onStart(cpt->getHandle(), msg->_id, msg->_buffer);
        break;
    case network::socketMessageType::M_SOCKET_ERROR:
        socketApi::onError(cpt->getHandle(), msg->_id, msg->_buffer);
        break;
    case network::socketMessageType::M_SOCKET_WARNING:
        socketApi::onWarning(cpt->getHandle(), msg->_id, msg->_ext);
        break;
    case network::socketMessageType::M_SOCKET_UDP:
        break;
    default:

        break;
    }

    if (isFree)
    {
        util::memory::free(msg->_buffer);
    }
    util::memory::free(msg);
}

boost::any socketApi::staticSocketUnPack(void *param, void *data, uint32_t size)
{
    boost::any result((network::socketMessage *)data);
    return result;
}

void socketApi::onAccept(uintptr_t opaque, int32_t id, int32_t clientId, char *addr)
{
    ptrSocket s = socketApi::getSocket(id);
    if (s == nullptr)
    {
        INST(socketSystem, doSocketClose, opaque, clientId);
        return;
    }

    s->acceptCallback(clientId, (const char *)addr);
}

void socketApi::onData(uintptr_t opaque, int32_t id, uint32_t size, char *data)
{
}

void socketApi::onStart(uintptr_t opaque, int32_t id, char *addr)
{
}

void socketApi::onError(uintptr_t opaque, int32_t id, char *err)
{
}

void socketApi::onWarning(uintptr_t opaque, int32_t id, int size)
{
}

void socketApi::onUdp(uintptr_t opaque, int32_t id, int size, char *data, char *addr)
{
}

void socketApi::onClose(uintptr_t opaque, int32_t id)
{
    ptrSocket s = socketApi::getSocket(id);
    if (s == nullptr)
    {
        return;
    }
    s->_connected = false;
}

ptrSocket socketApi::getSocket(int32_t id)
{
    std::unique_lock<util::spinlock> lock(m_socketMutex);
    if (m_socketPool.empty())
    {
        return nullptr;
    }

    auto it = m_socketPool.find(id);
    if (it == m_socketPool.end())
    {
        return nullptr;
    }
    return it->second;
}

void socketApi::doSuspend(ptrSocket s)
{
    assert(s->_co._id == 0);
    s->_so = operation::worker::suspend();
}

uint32_t socketApi::pushBuffer(wfSocketBuffer *ptrBuffer, char *data, uint32_t sz)
{
    wfBufferNode *freeNode = popBufferPool();
    assert(freeNode);

    freeNode->_data = data;
    freeNode->_sz = sz;

    if (ptrBuffer->_head == nullptr)
    {
        assert(ptrBuffer->_tail == nullptr);
        ptrBuffer->_head = ptrBuffer->_tail = freeNode;
    }
    else
    {
        ptrBuffer->_tail->_next = freeNode;
        ptrBuffer->_tail = freeNode;
    }

    ptrBuffer->_size += sz;

    return ptrBuffer->_size;
}

void socketApi::freeBufferNode(wfSocketBuffer *ptrBuffer)
{
    wfBufferNode *freeNode = ptrBuffer->_head;
    ptrBuffer->_offset = 0;
    ptrBuffer->_head = freeNode->_next;
    if (ptrBuffer->_head == nullptr)
    {
        ptrBuffer->_tail = nullptr;
    }

    util::memory::free(freeNode->_data);
    freeNode->_data = nullptr;
    freeNode->_sz = 0;
    pushBufferNode(freeNode);
}

void socketApi::pushBufferNode(wfBufferNode *ptrNode)
{
    std::unique_lock<util::spinlock> lock(m_bufferMutex);
    ptrNode->_next = m_bufferPool[0];
    m_bufferPool[0] = ptrNode;
}

wfBufferNode *socketApi::newBufferPool(int32_t num)
{
    wfBufferNode *pool = (wfBufferNode *)util::memory::malloc(sizeof(struct wfBufferNode) * num);
    assert(pool);
    for (int i = 0; i < num; i++)
    {
        pool[i]._data = NULL;
        pool[i]._sz = 0;
        pool[i]._next = &pool[i + 1];
    }
    pool[num - 1].next = NULL;
    return pool;
}

wfBufferNode *socketApi::popBufferPool()
{
    std::unique_lock<util::spinlock> lock(m_bufferMutex);
    wfBufferNode *freeNode = nullptr;
    if (!m_bufferPool.empty())
    {
        freeNode = m_bufferPool.front();
    }

    if (freeNode == nullptr)
    {
        size_t ztb = m_bufferPool.size();
        if (ztb == 0)
            ztb++;
        int size = 8;
        if (ztb <= SOCKET_LARGE_PAGE_NODE - 3)
        {
            size <<= tsz;
        }
        else
        {
            size <<= SOCKET_LARGE_PAGE_NODE - 3;
        }

        freeNode = newBufferPool(size);
        m_bufferPool.push(nullptr);
    }
    m_bufferPool[0] = freeNode->_next;
    freeNode->_next = nullptr;
    return freeNode;
}

} // namespace api
} // namespace wolf