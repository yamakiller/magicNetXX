#include "socket.h"
#include "module/message.h"
#include "network/socketSystem.h"
#include "util/memory.h"
#include "util/mobject.h"

#define SOCKET_BACKLOG 32
// 2 ** 12 == 4096
#define SOCKET_LARGE_PAGE_NODE 12
#define SOCKET_BUFFER_LIMIT (256 * 1024)

namespace wolf {
namespace api {

///////////////////////////////////////////////////
//基础数据结构不被外部访问
struct socketBufferNode {
  const char *_data;
  uint32_t _sz;
  socketBufferNode *_next;
};

struct socketBuffer {
  uint32_t _size;
  uint32_t _offset;
  struct socketBufferNode *_head;
  struct socketBufferNode *_tail;
};

class socketOper;
typedef std::shared_ptr<socketOper> ptrSocket;
typedef std::unordered_map<int32_t, ptrSocket> socket_maps;
typedef std::vector<socketBufferNode *> buffer_pools;

static socket_maps gSocketPool;
static util::spinlock gSocketMutex;

static buffer_pools gBufferPool;
static util::spinlock gBufferMutex;

static socketBufferNode *newBufferPool(int32_t num) {

  socketBufferNode *pool = (socketBufferNode *)util::memory::malloc(
      sizeof(struct socketBufferNode) * num);
  assert(pool);
  for (int i = 0; i < num; i++) {
    pool[i]._data = NULL;
    pool[i]._sz = 0;
    pool[i]._next = &pool[i + 1];
  }
  pool[num - 1]._next = NULL;
  return pool;
}

static socketBufferNode *popBufferPool() {
  std::unique_lock<util::spinlock> lock(gBufferMutex);
  socketBufferNode *freeNode = nullptr;
  if (!gBufferPool.empty()) {
    freeNode = gBufferPool.front();
  }

  if (freeNode == nullptr) {
    size_t ztb = gBufferPool.size();
    if (ztb == 0)
      ztb++;
    int size = 8;
    if (ztb <= SOCKET_LARGE_PAGE_NODE - 3) {
      size <<= ztb;
    } else {
      size <<= SOCKET_LARGE_PAGE_NODE - 3;
    }

    freeNode = newBufferPool(size);
    gBufferPool.push_back(nullptr);
  }
  gBufferPool[0] = freeNode->_next;
  freeNode->_next = nullptr;
  return freeNode;
}

void pushBufferNode(socketBufferNode *ptrNode) {
  std::unique_lock<util::spinlock> lock(gBufferMutex);
  ptrNode->_next = gBufferPool[0];
  gBufferPool[0] = ptrNode;
}

static void freeBufferNode(socketBuffer *ptrBuffer) {
  socketBufferNode *freeNode = ptrBuffer->_head;
  ptrBuffer->_offset = 0;
  ptrBuffer->_head = freeNode->_next;
  if (ptrBuffer->_head == nullptr) {
    ptrBuffer->_tail = nullptr;
  }

  util::memory::free((void *)freeNode->_data);
  freeNode->_data = nullptr;
  freeNode->_sz = 0;
  pushBufferNode(freeNode);
}

class socketOper : public util::mobject {
public:
  socketOper(aSharedPtr ptr, int32_t id, socketfunc func = nullptr) {
    m_id = id;
    m_opaque = ptr;
    m_func = func;
    m_connected = false;
    m_connecting = true;
    m_co.clear();
    m_closing.clear();
    m_readRequired.clear();
    memset(&m_buffer, 0, sizeof(socketBuffer));
    m_bufferLimit = 0;
  }

  ~socketOper() { clearBuffer(); }

  aSharedPtr getAssertPtr() {
    aSharedPtr ptr = m_opaque.lock();
    assert(ptr);
    return ptr;
  }

  bool isCoEmpty() { return m_co.empty(); }

  void doSuspend() {
    assert(m_co.empty());
    m_co = INST(module::actorSystem, doCreateCo);
    aSharedPtr ptr = getAssertPtr();
    ptr->getComponent()->doWait(boost::any_cast<module::coEntry>(m_co));

    if (!m_closing.empty()) {
      ptr->getComponent()->doWakeup(
          boost::any_cast<module::coEntry>(m_closing));
    }
  }

  void doCloseSusped(aSharedPtr ptr) {
    assert(m_closing.empty());
    m_closing = INST(module::actorSystem, doCreateCo);
    ptr->getComponent()->doWait(boost::any_cast<module::coEntry>(m_closing));
  }

  void doWakeup() {
    if (!m_co.empty()) {
      module::coEntry co = boost::any_cast<module::coEntry>(m_co);
      m_co.clear();
      aSharedPtr ptr = getAssertPtr();
      ptr->getComponent()->doWakeup(co);
    }
  }

  int32_t doRead(char *outBuffer, int outLen) {
    if (m_buffer._size < outLen || outLen == 0) {
      return -1;
    }
    popBuffer(outBuffer, outLen);
    m_buffer._size -= outLen;
    return outLen;
  }

  boost::any doReadLine(bool check, std::string sep) {
    struct socketBufferNode *current = m_buffer._head;
    if (current == nullptr) {
      return boost::any();
    }

    int from = m_buffer._offset;
    int bytes = current->_sz - from;
    size_t seplen = sep.length();
    for (int i = 0; i <= m_buffer._size - (int)seplen; i++) {
      if (checkSep(current, from, sep.c_str(), seplen)) {
        if (check) {
          return boost::any(true);
        } else {
          std::string sr = popString(i + seplen, seplen);
          m_buffer._size -= i + seplen;
          return sr;
        }
      }
      ++from;
      --bytes;
      if (bytes == 0) {
        current = current->_next;
        from = 0;
        if (current == nullptr) {
          break;
        }
        bytes = current->_sz;
      }
    }
    return boost::any();
  }

  std::string doReadAll() {
    std::string b;
    while (m_buffer._head) {
      struct socketBufferNode *current = m_buffer._head;
      b.append(current->_data + m_buffer._offset,
               current->_sz - m_buffer._offset);
      freeBufferNode(&m_buffer);
    }
    m_buffer._size = 0;
    return b;
  }

  int32_t getBufferLimit() { return m_bufferLimit; }
  void setBufferLimit(int32_t val) { m_bufferLimit = val; }

  uint32_t getDataSize() { return m_buffer._size; }

  void setConnected(bool val) { m_connected = val; }
  bool getConnected() { return m_connected; }

  bool isDisconnected() { return !(m_connected || !m_connecting.empty()); }

  bool isConnecting() { return !m_connecting.empty(); }

  boost::any getConnecting() { return m_connecting; }

  void clearConnecting() { m_connecting.clear(); }

  void setConnecting(boost::any val) { m_connecting = val; }

  boost::any getReadRequired() { return m_readRequired; }
  void setReadRequired(boost::any val) { m_readRequired = val; }
  void clearReadRequired() { m_readRequired.clear(); }

  void callback(int32_t id, const char *addr) { m_func(id, addr); }

  uint32_t pushData(char *data, uint32_t sz) {
    socketBufferNode *freeNode = popBufferPool();
    assert(freeNode);

    freeNode->_data = data;
    freeNode->_sz = sz;

    if (m_buffer._head == nullptr) {
      assert(m_buffer._tail == nullptr);
      m_buffer._head = m_buffer._tail = freeNode;
    } else {
      m_buffer._tail->_next = freeNode;
      m_buffer._tail = freeNode;
    }

    m_buffer._size += sz;

    return m_buffer._size;
  }

  void clearBuffer() {
    while (m_buffer._head) {
      freeBufferNode(&m_buffer);
    }

    m_buffer._size = 0;
  }

private:
  bool checkSep(struct socketBufferNode *node, int from, const char *sep,
                int seplen) {
    for (;;) {
      int sz = node->_sz - from;
      if (sz >= seplen) {
        return memcmp(node->_data + from, sep, seplen) == 0;
      }
      if (sz > 0) {
        if (memcmp(node->_data + from, sep, sz)) {
          return false;
        }
      }
      node = node->_next;
      sep += sz;
      seplen -= sz;
      from = 0;
    }
  }

  std::string popString(int sz, int skip) {
    struct socketBufferNode *current = m_buffer._head;
    if (sz < current->_sz - m_buffer._offset) {
      int tmpoffset = m_buffer._offset;
      m_buffer._offset += sz;
      return std::string(current->_data + tmpoffset, sz - skip);
    }

    if (sz == current->_sz - m_buffer._offset) {
      std::string rstr(current->_data + m_buffer._offset, sz - skip);
      freeBufferNode(&m_buffer);
      return rstr;
    }

    std::string b;
    for (;;) {
      int bytes = current->_sz - m_buffer._offset;
      if (bytes >= sz) {
        if (sz > skip) {
          b.append(current->_data + m_buffer._offset, sz - skip);
        }
        m_buffer._offset += sz;
        if (bytes == sz) {
          freeBufferNode(&m_buffer);
        }
        break;
      }
      int realSz = sz - skip;
      if (realSz > 0) {
        b.append(current->_data + m_buffer._offset,
                 (realSz < bytes) ? realSz : bytes);
      }

      freeBufferNode(&m_buffer);
      sz -= bytes;
      if (sz == 0) {
        break;
      }
      current = m_buffer._head;
      assert(current);
    }
    return b;
  }

  int32_t popBuffer(char *outBuffer, int outLen) {
    struct socketBufferNode *current = m_buffer._head;
    if (outLen < current->_sz - m_buffer._offset) {
      memcpy(outBuffer, current->_data + m_buffer._offset, outLen);
      m_buffer._offset += outLen;
      return outLen;
    }

    if (outLen == current->_sz - m_buffer._offset) {
      freeBufferNode(&m_buffer);
      return outLen;
    }

    int offset = 0;
    int sz = outLen;
    for (;;) {
      int bytes = current->_sz - m_buffer._offset;
      if (bytes >= sz) {
        memcpy(outBuffer + offset, current->_data + m_buffer._offset, sz);
        m_buffer._offset += sz;
        if (bytes == sz) {
          freeBufferNode(&m_buffer);
        }
        break;
      }

      memcpy(outBuffer + offset, current->_data + m_buffer._offset, bytes);

      freeBufferNode(&m_buffer);
      sz -= bytes;
      offset += bytes;
      if (sz == 0) {
        break;
      }
      current = m_buffer._head;
      assert(current);
    }

    return outLen;
  }

private:
  int32_t m_id;
  bool m_connected;
  boost::any m_connecting;
  boost::any m_readRequired;
  socketBuffer m_buffer;
  int32_t m_bufferLimit;
  boost::any m_co;
  boost::any m_closing;
  std::weak_ptr<module::actor> m_opaque;
  socketfunc m_func;
};

static boost::any makeSocket(aSharedPtr ptr, int32_t id, socketfunc func);
static ptrSocket getSocket(int32_t id);
static void addSocket(int32_t id, ptrSocket);
static void removeSocket(int32_t id);

static void staticSocketDispatch(void *param, int32_t session, uint32_t src,
                                 boost::any data);
static boost::any staticSocketUnPack(void *param, void *data, uint32_t size);

static void onAccept(uintptr_t opaque, int32_t id, int32_t clientId,
                     char *addr);
static void onData(uintptr_t opaque, int32_t id, uint32_t size, char *data);
static void onStart(int32_t id, char *addr);
static void onError(uintptr_t opaque, int32_t id, char *err);
static void onWarning(int32_t id, int size);
static void onUdp(uintptr_t opaque, int32_t id, int size, char *data,
                  char *addr);
static void onClose(int32_t id);

boost::any makeSocket(aSharedPtr ptr, int32_t id, socketfunc func) {
  ptrSocket s(new socketOper(ptr, id, func)); //继承于mobject
  assert(s);

  addSocket(id, s);

  s->doSuspend();
  boost::any err = s->getConnecting();
  s->clearConnecting();
  if (s->getConnected()) {
    return id;
  } else {
    removeSocket(id);
    if (err.type() == typeid(std::string)) {
      return err;
    }
    return -1;
  }
}

ptrSocket getSocket(int32_t id) {
  std::unique_lock<util::spinlock> lock(gSocketMutex);
  if (gSocketPool.empty()) {
    return nullptr;
  }

  auto it = gSocketPool.find(id);
  if (it == gSocketPool.end()) {
    return nullptr;
  }
  return it->second;
}

void addSocket(int32_t id, ptrSocket ptr) {
  std::unique_lock<util::spinlock> lock(gSocketMutex);
  gSocketPool[id] = ptr;
}

void removeSocket(int32_t id) {
  std::unique_lock<util::spinlock> lock(gSocketMutex);
  if (gSocketPool.empty()) {
    return;
  }

  auto it = gSocketPool.find(id);
  if (it == gSocketPool.end()) {
    return;
  }
  gSocketPool.erase(it);
}

void staticSocketDispatch(void *param, int32_t session, uint32_t src,
                          boost::any data) {
  int isFree = true;
  module::actorComponent *cpt = static_cast<module::actorComponent *>(param);
  network::socketMessage *msg = boost::any_cast<network::socketMessage *>(data);
  switch (msg->_type) {
  case network::socketMessageType::M_SOCKET_ACCEPT:
    onAccept(cpt->getHandle(), msg->_id, msg->_ext, msg->_buffer);
    break;
  case network::socketMessageType::M_SOCKET_DATA:
    onData(cpt->getHandle(), msg->_id, msg->_ext, msg->_buffer);
    isFree = false;
    break;
  case network::socketMessageType::M_SOCKET_CLOSE:
    onClose(msg->_id);
    break;
  case network::socketMessageType::M_SOCKET_START:
    onStart(msg->_id, msg->_buffer);
    break;
  case network::socketMessageType::M_SOCKET_ERROR:
    onError(cpt->getHandle(), msg->_id, msg->_buffer);
    break;
  case network::socketMessageType::M_SOCKET_WARNING:
    onWarning(msg->_id, msg->_ext);
    break;
  case network::socketMessageType::M_SOCKET_UDP:
    break;
  default:

    break;
  }

  if (isFree) {
    util::memory::free(msg->_buffer);
  }
  util::memory::free((void *)msg);
}

boost::any staticSocketUnPack(void *param, void *data, uint32_t size) {
  boost::any result((network::socketMessage *)data);
  return result;
}

void onAccept(uintptr_t opaque, int32_t id, int32_t clientId, char *addr) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    INST(network::socketSystem, doSocketClose, opaque, clientId);
    return;
  }

  s->callback(clientId, (const char *)addr);
}

void onData(uintptr_t opaque, int32_t id, uint32_t size, char *data) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    SYSLOG_ERROR(opaque, "Socket: drop package from {}", id);
    util::memory::free((void *)data);
    return;
  }

  uint32_t sz = s->pushData(data, size);
  boost::any rr = s->getReadRequired();
  if (!rr.empty() && rr.type() == typeid(int)) {
    int tmpsz = boost::any_cast<int>(rr);
    if (sz >= tmpsz) {
      s->clearReadRequired();
      s->doWakeup();
    }
  } else {
    if (s->getBufferLimit() && sz > s->getBufferLimit()) {
      SYSLOG_ERROR(opaque, "Socket Buffer Overflow: fd={} size={}", id, sz);
      s->clearBuffer();
      INST(network::socketSystem, doSocketClose, opaque, id);
      return;
    }
    if (!rr.empty() && rr.type() == typeid(std::string)) {
      if (!s->doReadLine(true, boost::any_cast<std::string>(rr)).empty()) {
        s->clearReadRequired();
        s->doWakeup();
      }
    }
  }
}

void onStart(int32_t id, char *addr) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    return;
  }

  s->setConnected(true);
  s->doWakeup();
}

void onError(uintptr_t opaque, int32_t id, char *err) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    SYSLOG_ERROR(opaque, "Socket: error on unknown {} {}", id, err);
    return;
  }

  if (s->getConnected()) {
    SYSLOG_ERROR(opaque, "Socket: error on {} {}", id, err);
  } else if (s->isConnecting()) {
    s->setConnecting(std::string(err));
  }
  s->setConnected(false);
  INST(network::socketSystem, doSocketShutdown, opaque, id);
  s->doWakeup();
}

void onWarning(int32_t id, int size) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    return;
  }

  aSharedPtr ptr = s->getAssertPtr();
  SYSLOG_ERROR(ptr->handle(), "WARNING: {} K bytes need to send out (fd = {})",
               size, id);
}

void onUdp(uintptr_t opaque, int32_t id, int size, char *data, char *addr) {}

void onClose(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    return;
  }
  s->setConnected(false);
  s->doWakeup();
}

/*外部调用函数**************************************************************************************************************/
void socketApi::doRequire(module::actorComponent *cpt) {
  cpt->doRegisterProtocol({module::messageId::M_ID_SOCKET,
                           &staticSocketDispatch, nullptr,
                           &staticSocketUnPack});
}

int32_t socketApi::doListen(std::shared_ptr<module::actor> ptr,
                            const char *addr, int port, int backlog) {
  return INST(network::socketSystem, doSocketListen, ptr->handle(), addr, port,
              backlog);
}

boost::any socketApi::doConnection(aSharedPtr ptr, const char *addr, int port) {
  int32_t id =
      INST(network::socketSystem, doSocketConnect, ptr->handle(), addr, port);
  return makeSocket(ptr, id, nullptr);
}

boost::any socketApi::doStart(aSharedPtr ptr, int32_t id, socketfunc func) {
  INST(network::socketSystem, doSocketStart, ptr->handle(), id);
  return makeSocket(ptr, id, func);
}

int32_t socketApi::doSend(int32_t id, char *data, uint32_t len) {
  return INST(network::socketSystem, doSocketSend, id, data, len);
}

int32_t socketApi::doRead(int32_t id, char *outBuffer, int outLen) {
  ptrSocket s = getSocket(id);
  assert(s);

  int32_t ret = s->doRead(outBuffer, outLen);
  if (ret != -1) {
    return ret;
  }

  if (!s->getConnected()) {
    return -1;
  }

  assert(s->getReadRequired().empty());
  s->setReadRequired(outLen);
  s->doSuspend();
  ret = s->doRead(outBuffer, outLen);
  if (ret != -1) {
    return ret;
  } else {
    return s->doRead(outBuffer, s->getDataSize());
  }
}

boost::any socketApi::doReadLine(int32_t id, std::string sep) {
  sep = sep.empty() ? "\n" : sep;
  ptrSocket s = getSocket(id);
  assert(s);
  boost::any ret = s->doReadLine(false, sep);
  if (!ret.empty()) {
    return boost::any_cast<std::string>(ret);
  }

  if (!s->getConnected()) {
    return stringResult({false, s->doReadAll()});
  }

  assert(s->getReadRequired().empty());
  s->setReadRequired(sep);
  s->doSuspend();
  if (s->getConnected()) {
    return s->doReadLine(false, sep);
  } else {
    return stringResult({false, s->doReadAll()});
  }
}

void socketApi::doSetNodelay(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    return;
  }

  INST(network::socketSystem, doSocketNodelay, id);
  // TODO  其他消息
}

void socketApi::doClose(aSharedPtr ptr, int32_t id) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    return;
  }

  if (s->getConnected()) {
    INST(network::socketSystem, doSocketClose, ptr->handle(), id);
    if (!s->isCoEmpty()) {
      s->doCloseSusped(ptr);
    } else {
      s->doSuspend();
    }
    s->setConnected(false);
  }
  s->clearBuffer();
  removeSocket(id);
}

void socketApi::doSetLimit(int32_t id, int limit) {
  ptrSocket s = getSocket(id);
  assert(s);
  s->setBufferLimit(limit);
}

bool socketApi::doBlock(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s == nullptr || !s->getConnected()) {
    return false;
  }
  assert(s->getReadRequired().empty());
  s->clearReadRequired();
  return s->getConnected();
}

int32_t socketApi::doGetDataSize(int32_t id) {
  ptrSocket s = getSocket(id);
  assert(s);
  return s->getDataSize();
}

bool socketApi::doDisconnected(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s != nullptr) {
    return s->isDisconnected();
  }
  return true;
}

void socketApi::doShutdown(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s != nullptr) {
    aSharedPtr ptr = s->getAssertPtr();
    s->clearBuffer();
    INST(network::socketSystem, doSocketShutdown, ptr->handle(), id);
  }
}

} // namespace api
} // namespace wolf