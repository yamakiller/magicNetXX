#include "socket.h"
#include "module/message.h"
#include "network/socketSystem.h"
#include "util/memory.h"

namespace wolf {
namespace api {
socket_maps socketApi::m_socketPool;
util::spinlock socketApi::m_socketMutex;

buffer_pools socketApi::m_bufferPool;
util::spinlock socketApi::m_bufferMutex;

bool checkSep(struct wfBufferNode *node, int from, const char *sep,
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

std::string wfSocket::popString(int sz, int skip) {
  struct wfBufferNode *current = _buffer._head;
  if (sz < current->_sz - _buffer._offset) {
    int tmpoffset = _buffer._offset;
    _buffer._offset += sz;
    return std::string(current->_data + tmpoffset, sz - skip);
  }

  if (sz == current->_sz - _buffer._offset) {
    std::string rstr(current->_data + _buffer._offset, sz - skip);
    socketApi::freeBuffer(&_buffer);
    return std::move(rstr);
  }

  std::string b;
  for (;;) {
    int bytes = current->_sz - _buffer._offset;
    if (bytes >= sz) {
      if (sz > skip) {
        b.append(current->_data + _buffer._offset, sz - skip);
      }
      _buffer._offset += sz;
      if (bytes == sz) {
        socketApi::freeBuffer(&_buffer);
      }
      break;
    }
    int realSz = sz - skip;
    if (realSz > 0) {
      b.append(current->_data + _buffer._offset,
               (realSz < bytes) ? realSz : bytes);
    }
    socketApi::freeBuffer(&_buffer);
    sz -= bytes;
    if (sz == 0) {
      break;
    }
    current = _buffer._head;
    assert(current);
  }
  return std::move(b);
}

int32_t wfSocket::popBuffer(char *outBuffer, int outLen) {
  struct wfBufferNode *current = _buffer._head;
  if (outLen < current->_sz - _buffer._offset) {
    memcpy(outBuffer, current->_data + _buffer._offset, outLen);
    _buffer._offset += outLen;
    return outLen;
  }

  if (outLen == current->_sz - _buffer._offset) {
    socketApi::freeBufferNode(&_buffer);
    return outLen;
  }

  int offset = 0;
  int sz = outLen;
  for (;;) {
    int bytes = current->_sz - _buffer._offset;
    if (bytes >= sz) {
      memcpy(outBuffer + offset, current->_data + _buffer._offset, sz);
      _buffer._offset += sz;
      if (bytes == sz) {
        socketApi::freeBufferNode(&_buffer);
      }
      break;
    }

    memcpy(outBuffer + offset, current->_data + _buffer._offset, bytes);

    socketApi::freeBufferNode(&_buffer);
    sz -= bytes;
    offset += bytes;
    if (sz == 0) {
      break;
    }
    current = _buffer._head;
    assert(current);
  }

  return outLen;
}

int32_t wfSocket::doRead(char *outBuffer, int outLen) {
  if (_buffer._size < outLen || outLen == 0) {
    return -1;
  }
  popBuffer(outBuffer, outLen);
  _buffer._size -= outLen;
  return outLen;
}

boost::any wfSocket::doReadStrLine(bool check, std::string sep) {
  struct wfBufferNode *current = _buffer._head;
  if (current == nullptr) {
    return boost::any();
  }

  int from = _buffer._offset;
  int bytes = current->_sz - from;
  size_t seplen = sep.length();
  for (int i = 0; i <= _buffer._size - (int)seplen; i++) {
    if (checkSep(current, from, sep.c_str(), seplen)) {
      if (check) {
        return boost::any(true);
      } else {
        std::string sr = popString(i + seplen, seplen);
        _buffer._size -= i + seplen;
        return std::move(sr);
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

std::string wfSocket::doReadStrAll() {
  std::string b;
  while (_buffer._head) {
    struct wfBufferNode *current = _buffer._head;
    b.append(current->_data + _buffer._offset, current->_sz - _buffer._offset);
    socketApi::freeBuffer(&_buffer);
  }
  _buffer._size = 0;
  return b;
}

void socketApi::doRequire(module::actorComponent *cpt) {
  cpt->doRegisterProtocol({module::messageId::M_ID_SOCKET,
                           &socketApi::staticSocketDispatch, nullptr,
                           &socketApi::staticSocketUnPack});
}

void socketApi::staticSocketDispatch(void *param, int32_t session, uint32_t src,
                                     boost::any data) {
  int isFree = true;
  module::actorComponent *cpt = static_cast<module::actorComponent *>(param);
  network::socketMessage *msg = boost::any_cast<network::socketMessage *>(data);
  switch (msg->_type) {
  case network::socketMessageType::M_SOCKET_ACCEPT:
    socketApi::onAccept(cpt->getHandle(), msg->_id, msg->_ext, msg->_buffer);
    break;
  case network::socketMessageType::M_SOCKET_DATA:
    socketApi::onData(cpt->getHandle(), msg->_id, msg->_ext, msg->_buffer);
    isFree = false;
    break;
  case network::socketMessageType::M_SOCKET_CLOSE:
    socketApi::onClose(msg->_id);
    break;
  case network::socketMessageType::M_SOCKET_START:
    socketApi::onStart(msg->_id, msg->_buffer);
    break;
  case network::socketMessageType::M_SOCKET_ERROR:
    socketApi::onError(cpt->getHandle(), msg->_id, msg->_buffer);
    break;
  case network::socketMessageType::M_SOCKET_WARNING:
    socketApi::onWarning(msg->_id, msg->_ext);
    break;
  case network::socketMessageType::M_SOCKET_UDP:
    break;
  default:

    break;
  }

  if (isFree) {
    util::memory::free(msg->_buffer);
  }
  util::memory::free(msg);
}

boost::any socketApi::staticSocketUnPack(void *param, void *data,
                                         uint32_t size) {
  boost::any result((network::socketMessage *)data);
  return result;
}

int32_t socketApi::doListen(std::shared_ptr<module::actor> ptr,
                            const char *addr, int port, int backlog) {
  return INST(network::socketSystem, doSocketListen, ptr->handle(), addr, port,
              backlog);
}

boost::any socketApi::doConnection(std::shared_ptr<module::actor> ptr,
                                   const char *addr, int port) {
  int32_t id =
      INST(network::socketSystem, doSocketConnect, ptr->handle(), addr, port);
  return createSocket(ptr, id, nullptr);
}

boost::any socketApi::doStart(
    std::shared_ptr<module::actor> ptr, int32_t id,
    std::function<void(int32_t clientId, const char *addr)> func) {
  INST(network::socketSystem, doSocketStart, ptr->handle(), id);
  return createSocket(ptr, id, func);
}

void socketApi::doSetLimit(int32_t id, int limit) {
  ptrSocket s = getSocket(id);
  assert(s);
  s->doReadStrLine = limit;
}

bool socketApi::doBlock(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s == nullptr || !s->_connected) {
    return false;
  }
  assert(s->_readRequired.empty());
  s->_readRequired.clear();
  return s->_connected;
}

void socketApi::doClose(std::shared_ptr<module::actor> ptr, int32_t id) {
  ptrSocket s = getSocket(id);
  if (s == nullptr) {
    return;
  }

  if (s->_connected) {
    INST(network::socketSystem, doSocketClose, ptr->handle(), id);
    if (!s->_co.empty()) {
      assert(s->_closing.empty());
      s->_closing = module::coCreate();
      ptr->getComponent()->doWait(
          boost::any_cast<module::coEntry>(s->_closing));
    } else {
      doSuspend(s);
    }
    s->_connected = false;
  }
  freeBuffer(&s->_buffer);
  removeSocket(id);
}

int32_t socketApi::doGetDataSize(int32_t id) {
  ptrSocket s = getSocket(id);
  assert(s);
  return s->_buffer._size;
}

int32_t socketApi::doRead(int32_t id, char *outBuffer, int outLen) {
  ptrSocket s = getSocket(id);
  assert(s);

  int32_t ret = s->doRead(outBuffer, outLen);
  if (ret != -1) {
    return ret;
  }

  if (!s->_connected) {
    return -1;
  }

  assert(s->_readRequired.empty());
  s->_readRequired = outLen;
  doSuspend(s);
  ret = s->doRead(outBuffer, outLen);
  if (ret != -1) {
    return ret;
  } else {
    return s->doRead(outBuffer, s->_buffer._size);
  }
}

std::string socketApi::doReadLine(int32_t id, bool *ok, std::string sep) {
  sep = sep.empty() ? "\n" : sep;
  ptrSocket s = getSocket(id);
  assert(s);
  *ok = true;
  boost::any ret = s->doReadStrLine(false, sep);
  if (!ret.empty()) {
    return boost::any_cast<std::string>(ret);
  }

  if (!s->_connected) {
    *ok = false;
    return s->doReadStrAll();
  }

  assert(s->_readRequired.empty());
  s->_readRequired = sep;
  doSuspend(s);
  if (s->_connected) {
    return boost::any_cast<std::string>(s->doReadStrLine(false, sep));
  } else {
    *ok = false;
    return s->doReadStrAll();
  }
}

std::string socketApi::doReadAll(int32_t id) {
  ptrSocket s = getSocket(id);
  assert(s);
  if (!s->_connected) {
    return s->doReadStrAll();
  }
  assert(s->_readRequired.empty());
  s->_readRequired = true;
  doSuspend(s);
  assert(s->_connected == false);
  return s->doReadStrAll();
}

bool socketApi::doDisconnected(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s != nullptr) {
    return !(s->_connected || !s->_connecting.empty());
  }
  return true;
}

void socketApi::doShutdown(int32_t id) {
  ptrSocket s = getSocket(id);
  if (s != nullptr) {
    freeBuffer(&s->_buffer);
    std::shared_ptr<module::actor> ptr = s->_opaque.lock();
    assert(ptr);
    INST(network::socketSystem, doSocketShutdown, ptr->handle(), id);
  }
}

boost::any socketApi::createSocket(
    std::shared_ptr<module::actor> ptr, int32_t id,
    std::function<void(int32_t clientId, const char *addr)> func) {

  ptrSocket s(new wfSocket()); //继承于mobject
  assert(s);
  s->_id = id;
  memset(&s->_buffer, 0, sizeof(s->_buffer));
  s->_bufferLimit = 0;
  s->_connected = false;
  s->_connecting = true;
  s->_co.clear();
  s->_closing.clear();
  s->_readRequired.clear();
  s->_callback = func;
  s->_opaque = ptr;

  pushSocket(id, s);
  doSuspend(s);
  boost::any err = s->_connecting;
  s->_connecting.clear();
  if (s->_connected) {
    return id;
  } else {
    removeSocket(id);
    if (err.type() == typeid(std::string)) {
      return err;
    }
    return -1;
  }
}
void socketApi::onAccept(uintptr_t opaque, int32_t id, int32_t clientId,
                         char *addr) {
  ptrSocket s = socketApi::getSocket(id);
  if (s == nullptr) {
    INST(network::socketSystem, doSocketClose, opaque, clientId);
    return;
  }

  s->_callback(clientId, (const char *)addr);
}

void socketApi::onData(uintptr_t opaque, int32_t id, uint32_t size,
                       char *data) {
  ptrSocket s = socketApi::getSocket(id);
  if (s == nullptr) {
    SYSLOG_ERROR(opaque, "Socket: drop package from {}", id);
    util::memory::free((void *)data);
    return;
  }

  uint32_t sz = socketApi::pushBuffer(&s->_buffer, data, size);
  boost::any rr = s->_readRequired;
  if (!rr.empty() && rr.type() == typeid(int)) {
    int tmpsz = boost::any_cast<int>(rr);
    if (sz >= tmpsz) {
      s->_readRequired.clear();
      doWakeup(s);
    }
  } else {
    if (s->_bufferLimit && sz > s->_bufferLimit) {
      SYSLOG_ERROR(opaque, "Socket Buffer Overflow: fd={} size={}", id, sz);
      freeBuffer(&s->_buffer);
      INST(network::socketSystem, doSocketClose, opaque, id);
      return;
    }
    if (!rr.empty() && rr.type() == typeid(std::string)) {
      if (!s->doReadStrLine(true, boost::any_cast<std::string>(rr)).empty()) {
        s->_readRequired.clear();
        doWakeup(s);
      }
    }
  }
}

void socketApi::onStart(int32_t id, char *addr) {
  ptrSocket s = socketApi::getSocket(id);
  if (s == nullptr) {
    return;
  }

  s->_connected = true;
  doWakeup(s);
}

void socketApi::onError(uintptr_t opaque, int32_t id, char *err) {
  ptrSocket s = socketApi::getSocket(id);
  if (s == nullptr) {
    SYSLOG_ERROR(opaque, "Socket: error on unknown {} {}", id, err);
    return;
  }

  if (s->_connected) {
    SYSLOG_ERROR(opaque, "Socket: error on {} {}", id, err);
  } else if (!s->_connecting.empty()) {
    s->_connecting = std::string(err);
  }
  s->_connected = false;
  INST(network::socketSystem, doSocketShutdown, opaque, id);
  doWakeup(s);
}

void socketApi::onWarning(int32_t id, int size) {
  ptrSocket s = socketApi::getSocket(id);
  if (s == nullptr) {
    return;
  }

  std::shared_ptr<module::actor> ptr = s->_opaque.lock();
  assert(ptr);
  SYSLOG_ERROR(ptr->handle(), "WARNING: {} K bytes need to send out (fd = {})",
               size, id);
}

void socketApi::onUdp(uintptr_t opaque, int32_t id, int size, char *data,
                      char *addr) {}

void socketApi::onClose(int32_t id) {
  ptrSocket s = socketApi::getSocket(id);
  if (s == nullptr) {
    return;
  }
  s->_connected = false;
  doWakeup(s);
}

ptrSocket socketApi::getSocket(int32_t id) {
  std::unique_lock<util::spinlock> lock(m_socketMutex);
  if (m_socketPool.empty()) {
    return nullptr;
  }

  auto it = m_socketPool.find(id);
  if (it == m_socketPool.end()) {
    return nullptr;
  }
  return it->second;
}

void socketApi::pushSocket(int32_t id, ptrSocket ptr) {
  std::unique_lock<util::spinlock> lock(m_socketMutex);
  m_socketPool[id] = ptr;
}

void socketApi::removeSocket(int32_t id) {
  std::unique_lock<util::spinlock> lock(m_socketMutex);
  if (m_socketPool.empty()) {
    return;
  }

  auto it = m_socketPool.find(id);
  if (it == m_socketPool.end()) {
    return;
  }
  m_socketPool.erase(it);
}

void socketApi::doSuspend(ptrSocket s) {
  assert(s->_co.empty());
  s->_co = module::coCreate();
  std::shared_ptr<module::actor> ptr = s->_opaque.lock();
  assert(ptr != nullptr);
  ptr->getComponent()->doWait(boost::any_cast<module::coEntry>(s->_co));

  if (!s->_closing.empty()) {
    ptr->getComponent()->doWakeup(
        boost::any_cast<module::coEntry>(s->_closing));
  }
}

void socketApi::doWakeup(ptrSocket s) {
  if (!s->_co.empty()) {
    module::coEntry co = boost::any_cast<module::coEntry>(s->_co);
    s->_co.clear();
    std::shared_ptr<module::actor> ptr = s->_opaque.lock();
    assert(ptr != nullptr);
    ptr->getComponent()->doWakeup(co);
  }
}

uint32_t socketApi::pushBuffer(wfSocketBuffer *ptrBuffer, char *data,
                               uint32_t sz) {
  wfBufferNode *freeNode = popBufferPool();
  assert(freeNode);

  freeNode->_data = data;
  freeNode->_sz = sz;

  if (ptrBuffer->_head == nullptr) {
    assert(ptrBuffer->_tail == nullptr);
    ptrBuffer->_head = ptrBuffer->_tail = freeNode;
  } else {
    ptrBuffer->_tail->_next = freeNode;
    ptrBuffer->_tail = freeNode;
  }

  ptrBuffer->_size += sz;

  return ptrBuffer->_size;
}

void socketApi::freeBuffer(wfSocketBuffer *ptrBuffer) {
  if (ptrBuffer == nullptr) {
    return;
  }

  while (ptrBuffer->_head) {
    freeBufferNode(ptrBuffer);
  }

  ptrBuffer->_size = 0;
}

void socketApi::freeBufferNode(wfSocketBuffer *ptrBuffer) {
  wfBufferNode *freeNode = ptrBuffer->_head;
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

void socketApi::pushBufferNode(wfBufferNode *ptrNode) {
  std::unique_lock<util::spinlock> lock(m_bufferMutex);
  ptrNode->_next = m_bufferPool[0];
  m_bufferPool[0] = ptrNode;
}

wfBufferNode *socketApi::newBufferPool(int32_t num) {
  wfBufferNode *pool =
      (wfBufferNode *)util::memory::malloc(sizeof(struct wfBufferNode) * num);
  assert(pool);
  for (int i = 0; i < num; i++) {
    pool[i]._data = NULL;
    pool[i]._sz = 0;
    pool[i]._next = &pool[i + 1];
  }
  pool[num - 1]._next = NULL;
  return pool;
}

wfBufferNode *socketApi::popBufferPool() {
  std::unique_lock<util::spinlock> lock(m_bufferMutex);
  wfBufferNode *freeNode = nullptr;
  if (!m_bufferPool.empty()) {
    freeNode = m_bufferPool.front();
  }

  if (freeNode == nullptr) {
    size_t ztb = m_bufferPool.size();
    if (ztb == 0)
      ztb++;
    int size = 8;
    if (ztb <= SOCKET_LARGE_PAGE_NODE - 3) {
      size <<= ztb;
    } else {
      size <<= SOCKET_LARGE_PAGE_NODE - 3;
    }

    freeNode = newBufferPool(size);
    m_bufferPool.push_back(nullptr);
  }
  m_bufferPool[0] = freeNode->_next;
  freeNode->_next = nullptr;
  return freeNode;
}

} // namespace api
} // namespace wolf