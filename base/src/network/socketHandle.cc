#include "socketHandle.h"
#include <string.h>

namespace engine {
namespace network {

static void sendBufferFree(sendData *p) {
  util::memory::free(p->_data);
  util::memcpy::free(p);
}

socketHandle::socketHandle()
    : m_handle(SOCKET_HANDLE_INVALID), m_opaque(0), m_sock(INVALID_SOCKET),
      m_sendIn(0), m_sendBytes(0), m_proto(socketProtocol::UNKNOWN),
      m_state(socketState::INVALID) {
  memset(&m_curSender, 0, sizeof(sendData));
  memset(&m_info, 0, sizeof(socketInfo));
}

socketHandle::~socketHandle() {}

void socketHandle::doRest() {
  m_handle = SOCKET_HANDLE_INVALID;
  m_opaque = 0;
  // m_sock = INVALID_SOCKET;
  m_sendIn = 0;
  m_sendBytes = 0;
  m_proto = socketProtocol::UNKNOWN;
  m_state = socketState::INVALID;

  memset(&m_curSender, 0, sizeof(sendData));
  memset(&m_info, 0, sizeof(socketInfo));
}

void socketHandle::doClose() {}

int32_t socketHandle::doSend() {
  if (m_curSender._data) {
    struct sendData *buf =
        (struct sendData *)util::memory::malloc(sizeof(sendData));
    buf->_data = m_curSender._data;
    buf->_ptr = m_curSender._ptr;
    buf->_sz = m_curSender._sz;

    m_sendQs.push_front(buf);
    memset(&m_curSender, 0, sizeof(m_curSender));
  }

  while (!m_sendQs.empty()) {
    struct sendData *tmp = m_sendQs.front();

    for (;;) {
      ssize_t sz = socketWrap::send(m_sock, tmp->_ptr,
                                    tmp->_sz - (tmp->_ptr - tmp->_data));
      if (sz < 0) {
        switch (errno) {
        case EINTR:
          continue;
        case AGAIN_WOULDBLOCK:
          return -1;
        }

        // TODO: onClose
        // force_close(ss, s, l, result);
        return MESSAGE_SOCKET_CLOSE;
      }

      m_sendBytes -= sz;
      tmp->_ptr += sz;
      if (tmp->_data + tmp->_sz != tmp->ptr) {
        return -1;
      }
      break;
    }
    m_sendQs.pop_front();
    sendBufferFree(tmp);
  }
  return -1;
}

void socketHandle::doUpdateRecv(uint64_t now, uint64_t bytes) {
  m_info._recvLastTime = now;
  m_info._recvBytes += bytes;
}

bool socketHandle::isSending() {
  return m_curSender._data != nullptr && !m_sendQs.empty() &&
         ((m_sendIn && 0xFFFF) != 0);
}

int32_t socketHandle::incSendIn() {
  if (m_proto != socketProtocol::TCP) {
    return 0;
  }

  for (;;) {
    uint32_t sendIn = m_sendIn;
    if ((sendIn >> MAX_SOCKET_P) == (uint32_t)SOCKET_HANDLE_TAG16(m_handle)) {
      if ((sendIn & 0xffff) == 0xffff) {
        continue;
      }

      if (__sync_bool_compare_and_swap(&m_sendIn, , sendIn, sendIn + 1)) {
        return 0;
      }
    } else {
      return -1;
    }
  }
}

void socketHandle::decSendIn() {
  if (m_proto != socketProtocol::TCP) {
    return;
  }

  assert((m_sendIn & 0xffff) != 0);
  __sync_sub_and_fetch(&m_sendIn, 1);
}

bool socketHandle::isCanSend(int32_t handle) {
  return m_handle == handle && !isSending() &&
         m_state == socketState::CONNECTED;
}

void socketHandle::appendSendQs(const char *data, const int32_t sz) {
  struct sendData *buf =
      (struct sendData *)util::memory::malloc(sizeof(sendData));
  buf->_data = (char *)buffer;
  buf->_ptr = (char *)buffer;
  buf->_sz = sz;

  m_sendQs.push_back(buf);
  m_sendBytes += sz;
}

void socketHandle::setHandle(int32_t val) {
  m_handle = val;
  m_sendIn = SOCKET_HANDLE_TAG16(m_handle) << 16 | 0;
}

int32_t socketHandle::getHandle() const { return m_handle; }

void socketHandle::setOpaque(uintptr_t val) { m_opaque = val; }

uintptr_t socketHandle::getOpaque() const { return m_opaque; }

void socketHandle::setSocket(wsocket_t val) { m_sock = val; }

wsocket_t socketHandle::getSocket() { return m_sock; }

void socketHandle::setProtocol(socketProtocol val) { m_proto = val; }

socketProtocol socketHandle::getProtocol() const { return m_proto; }

void socketHandle::setState(socketState val) { m_state = val; }

socketState socketHandle::getState() const { return m_state; }

bool socketHandle::setCompareState(socketState cval, socketState nval) {
  return __sync_bool_compare_and_swap(&m_state, cval, nval);
}

util::spinlock *socketHandle::getMutexRef() { return &m_mutex; }

socketHandle::socketInfo socketHandle::getInfo() const { return m_info; }

} // namespace network
} // namespace engine