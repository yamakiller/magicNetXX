#include "socketHandle.h"
#include "errorWrap.h"
#include "util/memory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

NS_CC_N_BEGIN

void socketHandle::sendBufferFree(sendData *p) {
  util::memory::free(p->_data);
  util::memory::free(p);
}

socketHandle::socketHandle()
    : m_handle(SOCKET_HANDLE_INVALID), m_opaque(0), m_sock(INVALID_SOCKET),
      m_sendIn(0), m_sendBytes(0), m_proto(socketProtocol::UNKNOWN),
      m_state(socketState::INVALID) {
  memset(&m_curSender, 0, sizeof(sendData));
  memset(&m_info, 0, sizeof(socketInfo));
}

socketHandle::~socketHandle() {}

void socketHandle::doInit(uintptr_t opaque, int32_t handle, wsocket_t sock,
                          socketProtocol proto) {
  m_handle = handle;
  m_opaque = opaque;
  m_sock = sock;
  m_proto = proto;
  m_sendBytes = 0;
  m_sendWarn = WARNING_SIZE;
  m_recvBufferSize = SOCKET_HANDLE_MINRECVBUFFER_MAX;
  m_sendIn = SOCKET_HANDLE_TAG16(m_handle) << 16 | 0;
}

void socketHandle::doRest() {
  m_handle = SOCKET_HANDLE_INVALID;
  m_opaque = 0;
  m_sock = INVALID_SOCKET;
  m_sendIn = 0;
  m_sendBytes = 0;
  m_sendWarn = WARNING_SIZE;
  m_proto = socketProtocol::UNKNOWN;
  m_state = socketState::INVALID;

  memset(&m_curSender, 0, sizeof(sendData));
  memset(&m_info, 0, sizeof(socketInfo));
}

void socketHandle::doClose() {
  while (!m_sendQs.empty()) {
    sendData *pd = m_sendQs.front();
    m_sendQs.pop_front();
    sendBufferFree(pd);
  }

  if (m_state != socketState::BIND) {
    if (socketWrap::close(m_sock) < 0) {
      perror("close socket!");
    }
  }

  m_state = socketState::INVALID;
  if (m_curSender._data) {
    util::memory::free(m_curSender._data);
    m_curSender._data = nullptr;
  }
}

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
      ssize_t sz = socketWrap::send(
          m_sock, tmp->_ptr, tmp->_sz - (tmp->_ptr - (char *)tmp->_data));
      if (sz < 0) {
        switch (errorWrap::wsalasterror()) {
        case EINTR:
          continue;
        case AGAIN_WOULDBLOCK:
          return -1;
        }

        // TODO: onClose 什么都不做
        return (int32_t)socketMessageType::M_SOCKET_CLOSE;
      }

      m_sendBytes -= sz;
      tmp->_ptr += sz;
      if ((char *)tmp->_data + tmp->_sz != tmp->_ptr) {
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

      if (__sync_bool_compare_and_swap(&m_sendIn, sendIn, sendIn + 1)) {
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
  buf->_data = (char *)data;
  buf->_ptr = (char *)data;
  buf->_sz = sz;

  m_sendQs.push_back(buf);
  m_sendBytes += sz;
}

void socketHandle::setHandle(int32_t val) {
  m_handle = val;
  m_sendIn = SOCKET_HANDLE_TAG16(m_handle) << 16 | 0;
}

int32_t socketHandle::getHandle() const { return m_handle; }

uintptr_t socketHandle::getOpaque() const { return m_opaque; }

wsocket_t socketHandle::getSocket() { return m_sock; }

uint32_t socketHandle::getWarning() {
  if (m_sendBytes > WARNING_SIZE && m_sendBytes > m_sendWarn) {
    m_sendWarn = m_sendWarn == 0 ? WARNING_SIZE * 2 : m_sendWarn * 2;
    return m_sendBytes % 1024 == 0 ? m_sendBytes / 1024
                                   : m_sendBytes / 1024 + 1;
  } else {
    return 0;
  }
}

socketProtocol socketHandle::getProtocol() const { return m_proto; }

void socketHandle::setState(socketState val) { m_state = val; }

socketState socketHandle::getState() const { return m_state; }

bool socketHandle::setCompareState(socketState cval, socketState nval) {
  return __sync_bool_compare_and_swap(&m_state, cval, nval);
}

util::spinlock *socketHandle::getMutexRef() { return &m_mutex; }

socketHandle::socketInfo socketHandle::getInfo() const { return m_info; }

NS_CC_M_END