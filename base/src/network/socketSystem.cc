#include "socketSystem.h"

#ifdef UT_PLATFORM_LINUX
#include "iocpEpoll.h"
#elif defined(UT_PLATFORM_APPLE)
#include "iocpKqueue.h"
#elif defined(UT_PLATFORM_WINDOW)
// TODO:
#endif

namespace engine {
namespace network {
socketSystem::socketSystem() {
  m_iocp = nullptr;
  m_channel = nullptr;
  m_shutdown = false;
}
socketSystem::~socketSystem() {}

int32_t socketSystem::doStart() {
#ifdef UT_PLATFORM_LINUX
  m_iocp = new iocpEpoll();
#elif defined(UT_PLATFORM_APPLE)
  m_iocp = new iocpKqueue();
#elif defined(UT_PLATFORM_WINDOW)
#endif
  assert(m_iocp);
  m_channel = new socketChannel();
  assert(m_channel);

  m_channel->doRegisterSendFunc(std::build(&socketSystem::doSendProccess, this,
                                           placeholders::_1, placeholders::_2));
  m_channel->doRegisterCloseFunc(
      std::bind(&socketSystem::doCloseProccess，this, placeholders::_1));

  std::thread t([this]() { this->doPoll(); });
  m_pid.swap(t);
  return 0;
}

void socketSystem::doShutdown() {
  m_shutdown = true;
  if (m_pid.joinable()) {
    m_pid.join();
  }
  delete m_channel;
  delete m_iocp;
}

int32_t socketSystem::doSocketListen(uintptr_t opaque, const char *addr,
                                     int32_t port, int32_t block) {

  int32_t handle;
  socketHandle *s = nullptr;
  wsocket_t sock = doSocketBind(addr, port, IPPROTO_TCP);
  if (sock == INVALID_SOCKET) {
    return SOCKET_HANDLE_INVALID;
  }

  if (socketWrap::listen(sock, block) != 0) {
    goto _FAILED;
  }

  handle = getReserve();
  if (handle == SOCKET_HANDLE_INVALID) {
    goto _FAILED;
  }

  s = getSocket(handle);
  assert(s);
  assert(s->setCompareState(socketState::RESERVE, socketState::RESERVE));
  std::unique_lock<util::spinlock> lock(*s->getMutexRef());

  s->setSocket(sock);
  s->setOpaque(opaque);
  s->setProtocol(socketProtocol::TCP);
  s->setState(socketState::PLISTEN);
  s->setRecvBufferSize(64);

  return handle;
_FAILED:
  socketWrap::close(handle);
  return SOCKET_HANDLE_INVALID;
}

int32_t socketSystem::doSocketOpen(const int32_t handle) {
  socketHandle *s = getSocket(handle);
  std::unique_lock<util::spinlock> lock(*s->getMutexRef());
  if (s->getState() == socketState::INVALID || s->getHandle() != handle) {
    return SOCKET_ERROR;
  }

  if (s->getState() == socketState::PACCEPT ||
      s->getState() == socketState::PLISTEN) {
    if (!m_iocp->doRegister(s->getSocket(), (void *)s)) {
      s->doClose();
      return SOCKET_ERROR;
    }

    s->setState((s->getState() == socketState::PACCEPT) ? socketState::CONNECTED
                                                        : socketState::LISTEN);
    return 0;
  } else if (s->getState() socketState::CONNECTED) {
    return 0;
  }
  return SOCKET_ERROR;
}

int32_t socketSystem::doSocketSend(int32_t handle, const char *data,
                                   size_t sz) {
  socketHandle *s = getSocket(handle);
  if (s->getState() == socketState::INVALID ||
      s->getState() == socketState::HALFCLOSE ||
      s->getState() == socketState::PACCEPT) {
    util::memory::free(data);
    return SOCKET_ERROR;
  }

  if (s->getState() == socketState::PLISTEN ||
      s->getState() == socketState::LISTEN) {
    fprintf(stderr, "Socket System: write to listen fd %d.\n", socketId);
    util::memory::free(data);
    return SOCKET_ERROR;
  }

  if (s->getMutexRef()->try_lock()) {
    if (s->isCanSend(handle)) {
      // TODO: 立即发送数据
      int n = socketWrap::send(s->getSocket(), data, sz);
      if (n < 0) {
        // ignore error, let socket thread try again
        n = 0;
      }

      if (n == so.sz) {
        s->getMutexRef()->unlock();
        util::memory::free(data);
        return 0;
      }

      sendData *curData = s->getCurrentSendBuffer();
      curData->_data = data;
      curData->_ptr = data + n;
      curData->_sz = sz;

      m_iocp->doToWrite(s->getSocket(), s, true);
      s->getMutexRef()->unlock();
      return 0;
    }
    s->getMutexRef()->unlock();
  }

  if (s->decSendIn() != 0) {
    memory::free(data);
    return SOCKET_ERROR;
  }

  socketChannel::requestPacket request;
  request.u.send._handle = handle;
  request.u.send._sz = sz;
  request.u.send._data = (char *)data;

  m_channel->doSend(&request, 'D', sizeof(request.u.send));
  return 0;
}

void socketSystem::doSocketClose(uintptr_t opaque, int32_t handle) {

  socketChannel::requestPacket request;
  request.u.close._handle = handle;
  request.u.close._shutdown = 0;
  request.u.close._opaque = opaque;

  m_channel->doSend(&request, 'K', sizeof(request.u.close));
}

void socketSystem::doSocketShutdown(uintptr_t opaque, int32_t handle) {

  socketChannel::requestPacket request;
  request.u.close._handle = handle;
  request.u.close._shutdown = 1;
  request.u.close._opaque = opaque;

  m_channel->doSend(&request, 'K', sizeof(request.u.close));
}

wsocket_t socketSystem::doSocketBind(const char *addr, int port, int protocol) {

  char strPort[16];
  struct addrinfo aiHints;
  struct addrinfo *aiList = nullptr;

  if (addr == NULL || addr[0] == 0) {
    addr = "0.0.0.0";
  }

  sprintf(strPort, "%d", port);
  memset(&ai_hints, 0, sizeof(ai_hints));

  aiHints.ai_family = AF_UNSPEC;
  if (protocol == IPPROTO_TCP) {
    aiHints.ai_socktype = SOCK_STREAM;
  } else {
    assert(protocol == IPPROTO_UDP);
    aiHints.ai_socktype = SOCK_DGRAM;
  }
  aiHints.ai_protocol = protocol;

  int status = ::getaddrinfo(addr, strPort, &aiHints, &aiList);
  if (status != 0) {
    return INVALID_SOCKET;
  }

  int family = aiList->ai_family;
  wsocket_t sock = ::socket(family, aiList->ai_socktype, 0);
  if (sock == INVALID_SOCKET) {
    goto _FAILED_FD;
  }

  if (socketWrap::reuseaddr(sock) == -1) {
    goto _FAILED;
  }

  status = socketWrap::bind(sock, (struct sockaddr *)aiList->ai_addr,
                            aiList->ai_addrlen);
  if (status != 0) {
    goto _FAILED;
  }

  freeaddrinfo(aiList);
  return sock;

_FAILED:
  socketWrap::close(sock);
_FAILED_FD:
  freeaddrinfo(aiList);
  return INVALID_SOCKET;
}

int32_t socketSystem::getReserve() {
  int i;
  for (i = 0; i < MAX_SOCKET; i++) {
    int32_t handle = ++m_sockSequence;
    if (handle < 0) {
      handle = (m_sockSequence &= 0x7FFFFFFF);
    }

    socketHandle *s = &m_sockGroup[SOCKET_GET(handle)];
    if (s->getState() == socketState::INVALID) {
      if (s->setCompareState(socketState::INVALID, socketState::RESERVE)) {
        s->doRest();
        s->setHandle(handle);
        return handle;
      } else {
        --i;
      }
    }
  }
  return -1;
}

socketHandle *socketSystem::getSocket(int32_t handle) {
  return &m_sockGroup[SOCKET_GET(handle)];
}

int32_t socketSystem::doSendProccess(socketChannel::requestSend *request,
                                     const uint8_t address) {
  int32_t handle = request->_handle;
  socketHandle *s = getSocket(handle);
  /* mutex */
  std::unique_lock<util::spinlock> lock(*s->getMutexRef());

  if (s->getState() == socketState::INVALID ||
      s->getState() == socketState::HALFCLOSE ||
      s->getState() == socketState::PACCEPT || s->getHandle() != handle) {
    util::memory::free(request->_data);
    return SOCKET_ERROR;
  }

  if (s->getState() == socketState::PLISTEN ||
      s->getState() == socketState::LISTEN) {
    fprintf(stderr, "Socket System: write to listen fd %d.\n", handle);
    util::memory::free(request->_data);
    return SOCKET_ERROR;
  }

  if (s->isSendQEmpty() && s->getState() == socketState::CONNECTED) {
    s->appendSendQs(request->_data, request->_sz);
    m_iocp->doToWrite(s->getSocket(), (void *)s, true);
  } else {
    s->appendSendQs(request->_data, request->_sz);
  }
  return SOCKET_ERROR;
}

int32_t socketSystem::doCloseProccess(socketChannel::requestClose *request) {
  int32_t handle = request->_handle;
  socketHandle *s = getSocket(handle);
  if (s->getState() == socketState::INVALID || s->getHandle() != handle) {
    // TODO 发送关闭信息
    return -1;
  }

  s->getMutexRef()->lock();
  if (s->isSending()) {
    int ret = s->doSend();
    if (ret == MESSAGE_SOCKET_CLOSE) {
      s->getMutexRef()->unlock();
      // TODO 发送关闭消息
      return -1;
    }
  }

  if (request->_shutdown || !s->isSending()) {
    // force_close(ss, s, &l, result);
    s->getMutexRef()->unlock();
    // TODO 发送关闭消息
    return -1;
  }
  s->getMutexRef()->unlock();
  s->setState(socketState::HALFCLOSE);
  return -1;
}

int32_t socketSystem::doConnectProccess(socketHandle *s) {
  int error;
  socklen_t len = sizeof(error);
  int code = getsockopt(s->getSocket(), SOL_SOCKET, SO_ERROR, &error, &len);
  if (code < 0 || error) {
    // force_close(ss, s, l, result);
    // if (code >= 0)
    //  result->data = strerror(error);
    // else
    //  result->data = strerror(errno);
    // TODO 发送错误消息
    return -1;
  } else {

    s->setState(socketState::CONNECTED);

    if (!s->isSending()) {
      m_iocp->doToWrite(s->getSocket(), s, false);
    }

    union socketAddr u;
    socklen_t slen = sizeof(u);
    if (getpeername(s->getSocket(), &u.s, &slen) == 0) {
      void *sin_addr = (u.s.sa_family == AF_INET) ? (void *)&u.v4.sin_addr
                                                  : (void *)&u.v6.sin6_addr;
      char buffer[64];
      if (inet_ntop(u.s.sa_family, sin_addr, buffer, sizeof(buffer))) {
        // TODO 设置客户端地址信息
        // TODO 发送OPEN事件
        return -1;
      }
    }
    result->data = NULL;
    // TODO 发送OPEN事件
    return -1;
  }
}

int32_t socketSystem::doAcceptProccess(socketHandle *s) {
  union socketAddr u;

  socklen_t len = sizeof(u);
  wsocket_t clientSock = ::accept(s->getSocket(), &u.s, &len);
  if (clientSock < 0) {
    if (errno == EMFILE || errno == ENFILE) {
      // TODO 发送错误信息
      return -1;
    } else {
      // TODO 需要关闭，或者
      return 0;
    }
  }

  int32_t handle = getReserve();
  if (handle < 0) {
    socketWrap::close(clientSock);
    return 0;
  }

  socketWrap::setKeepalive(clientSock);
  socketWrap::setNonblock(clientSock);
  socketHandle *cs = getSocket(handle);
  assert(cs->getState() == socketState::RESERVE);
  cs->setHandle(handle);
  cs->setSocket(clientSock);
  cs->setOpaque(socketProtocol::TCP);
  cs->setOpaque(s->getOpaque());
  cs->setRecvBufferSize(64);

  // TODO 设置客户端IP端口

  // TODO 发送ACCEPT事件
  return 0;
}

int32_t socketSystem::doRecvProccess(socketHandle *s) {
  int32_t handle = s->getHandle();
  uintptr_t opaque = s->getOpaque();

  int32_t bufferSize = s->getRecvBufferSize();
  char *buffer = (char *)util::memory::malloc(bufferSize);
  assert(buffer);
  int n = socketWrap::recv(s->getSocket(), buffer, bufferSize);
  if (n < 0) {
    util::memory::free(buffer);
    switch (errno) {
    case EINTR:
      break;
    case AGAIN_WOULDBLOCK:
      fprintf(stderr, "Socket System: EAGAIN capture.\n");
      break;
    default:
      // TODO: onClose(rl);
      return MESSAGE_SOCKET_CLOSE;
    }
    return -1;
  }

  if (n == 0) {
    util::memory::free(buffer);
    // TODO: onClose(rl);
    return MESSAGE_SOCKET_CLOSE;
  }
  if (s->getState() == socketState::HALFCLOSE) {
    util::memory::free(buffer);
    return -1;
  }

  if (n == sz) {
    bufferSize *= 2;
  } else if (sz > 64 && n * 2 > bufferSize) {
    bufferSize /= 2;
  }
  s->setRecvBufferSize(bufferSize);
  s->doUpdateRecv(0, n);

  // TODO: 推送数据
  memory::free(buffer);
  return MESSAGE_SOCKET_DATA;
}

int32_t socketSystem::doSendProccess(socketHandle *s) {
  if (!s->getMutexRef()->try_lock()) {
    return -1;
  }

  if (s->doSend() == MESSAGE_SOCKET_CLOSE) {
    s->getMutexRef()->unlock();
    // TODO 发送
    return -1;
  }

  if (s->isSendQEmpty()) {
    m_iocp->doToWrite(s->getSocket(), s, false);
  }

  if (s->getState() == socketState::HALFCLOSE) {
    // TODO: onClose
    // force_close(ss, s, l, result);
    s->getMutexRef()->unlock();
    // TODO: 发送
    return MESSAGE_SOCKET_CLOSE;
  }

  s->getMutexRef()->unlock();

  return -1;
}

void socketSystem::doPoll() {
  int32_t n, idx = 0;
  while (!m_shutdown) {

    int r = m_channel->doWait();
    if (r == -1) {
      if (m_channel->getError() == socketChannel::errCode::CH_EINTR) {
        continue;
      } else if (m_channel->getError() == socketChannel::errCode::CH_EXIT) {
        return;
      }
    }

    if (idx == n) {
      n = m_iocp->onWait();
      idx = 0;
      if (n < 0) {
        n = 0;
        if (errno == EINTR) {
          continue;
        }

        return;
      }
    }

    struct iocpEvent *e = m_iocp->getEvent(idx++);
    socketHandle *s = static_cast<socketHandle *>(e->s);
    if (s == nullptr) {
      continue;
    }

    //是否构建锁
    switch (s->getState()) {
    case socketState::CONNECTING:
      doConnectProccess(s);
      break;
    case socketState::LISTEN:
      doAcceptProccess(s);
      break;
    case socketState::INVALID:
      break;
    default:
      if (e->isread) {
        int type = doRecvProccess(s);
        if (e->iswrite && type == MESSAGE_SOCKET_CLOSE) {
          e->isread = false;
          --idx;
        }
        break;
      }
      if (e->iswrite) {
        doSendProccess(s);
        break;
      }
      if (e->iserror) {
      }
      if (e->iseof) {
      }
      break;
    }
  }
}

// int32_t socketSystem::doChannelRecv() {
/* data Piece */
//  uint8_t data[256];
/* [0 bit command][ 1 bit bytes] */
//  uint8_t header[2];
/*
  m_channel->doRecv(header, sizeof(header));
  int command = header[0];
  int bytes = header[1];

  m_channel->doRecv(data, bytes);

  switch (command) {
  case 'D':
  case 'P':
    break;
  case 'X':
    return -1;

  default:
    break;
  }
}*/

} // namespace network
} // namespace engine