#include "socketSystem.h"

#ifdef UT_PLATFORM_LINUX
#include "iocpEpoll.h"
#elif defined(UT_PLATFORM_APPLE)
#include "iocpKqueue.h"
#elif defined(UT_PLATFORM_WINDOW)
// TODO:
#endif
#include "errorWrap.h"
#include "module/actorSystem.h"
#include "module/message.h"
#include "operation/clock.h"
#include "util/stringUtil.h"

#define REG_SOCK_MESSAGE(type, func)                                           \
  doRegisterEvent(type,                                                        \
                  [](uintptr_t opaque, int32_t handle, int32_t ud, void *data, \
                     size_t sz) { func(opaque, handle, ud, data, sz); })

NS_CC_N_BEGIN

socketMessage *doMessageGen(int type, int32_t handle, void *data, size_t sz) {
  socketMessage *msg = (socketMessage *)util::memory::malloc(sizeof(*msg));
  assert(msg);

  if (data && sz == 0) {
    if (strcmp((const char *)data, "") == 0) {
      data = (char *)util::memory::malloc(4);
      assert(data);
      memset(data, 0, 4);
    } else {
      data = (void *)util::stringUtil::strdup((const char *)data);
    }
  }

  msg->_id = handle;
  msg->_type = type;
  msg->_buffer = (char *)data;

  return msg;
}

bool forwardMessage(uintptr_t opaque, socketMessage *msg) {
  if (INST(module::actorSystem, doSendMessage, 0, opaque,
           module::messageId::M_ID_SOCKET, 0, (void *)msg,
           sizeof(socketMessage)) != 0) {
    util::memory::free(msg->_buffer);
    util::memory::free((void *)msg);
    return false;
  }
  return true;
}

void onSocketAccept(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
                    size_t sz) {
  socketMessage *msg =
      doMessageGen(socketMessageType::M_SOCKET_ACCEPT, handle, data, sz);
  msg->_ext = ud;
  if (!forwardMessage(opaque, msg)) {
    SYSLOG_ERROR(opaque, "Push accept message failed Listen:{} Client:{}",
                 handle, ud);
  }
}

void onSocketStart(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
                   size_t sz) {
  socketMessage *msg =
      doMessageGen(socketMessageType::M_SOCKET_START, handle, data, sz);
  msg->_ext = 0;
  if (!forwardMessage(opaque, msg)) {
    SYSLOG_ERROR(opaque, "Push start message failed Socket:{},{}", handle,
                 data);
  }
}

void onSocketData(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
                  size_t sz) {
  socketMessage *msg =
      doMessageGen(socketMessageType::M_SOCKET_DATA, handle, data, sz);
  msg->_ext = sz;
  if (!forwardMessage(opaque, msg)) {
    SYSLOG_ERROR(opaque, "Push data message failed Socket:{},{}", handle, sz);
  }
}

void onSocketClose(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
                   size_t sz) {
  socketMessage *msg =
      doMessageGen(socketMessageType::M_SOCKET_CLOSE, handle, data, sz);
  msg->_ext = ud;
  if (!forwardMessage(opaque, msg)) {
    SYSLOG_ERROR(opaque, "Push close message failed Socket:{}", handle);
  }
}

void onSocketError(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
                   size_t sz) {
  socketMessage *msg =
      doMessageGen(socketMessageType::M_SOCKET_ERROR, handle, data, sz);
  msg->_ext = ud;
  if (!forwardMessage(opaque, msg)) {
    SYSLOG_ERROR(opaque, "Push error message failed Socket:{}{}", handle, data);
  }
}

void onSocketWarn(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
                  size_t sz) {
  socketMessage *msg =
      doMessageGen(socketMessageType::M_SOCKET_WARNING, handle, data, sz);
  msg->_ext = ud;
  if (!forwardMessage(opaque, msg)) {
    SYSLOG_ERROR(opaque, "Push warning message failed Socket:{}{}", handle, ud);
  }
}

socketSystem::socketSystem() {
  m_iocp = nullptr;
  m_channel = nullptr;
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

  m_channel->doRegisterConnectFunc(
      std::bind(&socketSystem::doRequestConnect, this, std::placeholders::_1));
  m_channel->doRegisterSendFunc(std::bind(&socketSystem::doRequestSend, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2));
  m_channel->doRegisterSetOptFunc(
      std::bind(&socketSystem::doRequestSetOpt, this, std::placeholders::_1));
  m_channel->doRegisterCloseFunc(
      std::bind(&socketSystem::doRequestClose, this, std::placeholders::_1));

  m_channel->doRegisterClearClosedFunc(std::bind(
      &socketSystem::doRequestClearClosedEvent, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  using smt = socketMessageType;
  REG_SOCK_MESSAGE(smt::M_SOCKET_ACCEPT, onSocketAccept);
  REG_SOCK_MESSAGE(smt::M_SOCKET_START, onSocketStart);
  REG_SOCK_MESSAGE(smt::M_SOCKET_DATA, onSocketData);
  REG_SOCK_MESSAGE(smt::M_SOCKET_ERROR, onSocketError);
  REG_SOCK_MESSAGE(smt::M_SOCKET_CLOSE, onSocketClose);
  REG_SOCK_MESSAGE(smt::M_SOCKET_WARNING, onSocketWarn);

  std::thread t([this]() { this->doPoll(); });
  m_pid.swap(t);
  return 0;
}

void socketSystem::doShutdown() {
  socketChannel::requestPacket request;
  request.u.exit._opaque = 0;
  m_channel->doSend(&request, 'X', sizeof(request.u.exit));

  if (m_pid.joinable()) {
    m_pid.join();
  }

  // TODO 销毁

  delete m_channel;
  delete m_iocp;
}

void socketSystem::doRegisterEvent(socketMessageType type,
                                   socketEventFunc Func) {
  m_eFunc[getEventFuncPos(type)] = Func;
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

  s->getMutexRef()->lock();
  s->doInit(opaque, handle, sock, socketProtocol::TCP);
  s->setState(socketState::PLISTEN);
  s->getMutexRef()->unlock();

  return handle;
_FAILED:
  socketWrap::close(handle);
  return SOCKET_HANDLE_INVALID;
}

int32_t socketSystem::doSocketConnect(uintptr_t opaque, const char *addr,
                                      int32_t port) {
  socketChannel::requestPacket request;
  int len = strlen(addr);
  if (len + sizeof(request.u.connect) >= 256) {
    fprintf(stderr, "Socket System : Invalid addr %s.\n", addr);
    return -1;
  }

  int32_t handle = getReserve();
  if (handle < 0) {
    return -1;
  }

  request.u.connect._opaque = opaque;
  request.u.connect._handle = handle;
  request.u.connect._port = port;
  memcpy(request.u.connect._addr, addr, len);
  request.u.connect._addr[len] = '\0';

  m_channel->doSend(&request, 'C', sizeof(request.u.connect) + len);

  return handle;
}

int32_t socketSystem::doSocketStart(uintptr_t opaque, const int32_t handle) {
  socketChannel::requestPacket request;
  request.u.start._handle = handle;
  request.u.start._opaque = opaque;

  m_channel->doSend(&request, 'S', sizeof(request.u.start));

  return 0;
}

int32_t socketSystem::doSocketSend(int32_t handle, const char *data,
                                   size_t sz) {
  socketHandle *s = getSocket(handle);
  if (s->getState() == socketState::INVALID ||
      s->getState() == socketState::HALFCLOSE ||
      s->getState() == socketState::PACCEPT) {
    util::memory::free((void *)data);
    return SOCKET_ERROR;
  }

  if (s->getState() == socketState::PLISTEN ||
      s->getState() == socketState::LISTEN) {
    fprintf(stderr, "Socket System: write to listen fd %d.\n", handle);
    util::memory::free((void *)data);
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

      if (n == sz) {
        s->getMutexRef()->unlock();
        util::memory::free((void *)data);
        return 0;
      }

      s->setCurrentSendBuffer((void *)data, (char *)data + n, sz);

      m_iocp->doToWrite(s->getSocket(), s, true);
      s->getMutexRef()->unlock();
      return 0;
    }
    s->getMutexRef()->unlock();
  }

  if (s->incSendIn() != 0) {
    util::memory::free((void *)data);
    return SOCKET_ERROR;
  }

  socketChannel::requestPacket request;
  request.u.send._handle = handle;
  request.u.send._sz = sz;
  request.u.send._data = (char *)data;

  m_channel->doSend(&request, 'D', sizeof(request.u.send));
  return 0;
}

int32_t socketSystem::doSocketNodelay(int32_t handle) {
  socketChannel::requestPacket request;
  request.u.setopt._handle = handle;
  request.u.setopt._what = TCP_NODELAY;
  request.u.setopt._value = 1;
  m_channel->doSend(&request, 'T', sizeof(request.u.setopt));
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
  memset(&aiHints, 0, sizeof(aiHints));

  aiHints.ai_family = AF_UNSPEC;
  if (protocol == IPPROTO_TCP) {
    aiHints.ai_socktype = SOCK_STREAM;
  } else {
    assert(protocol == IPPROTO_UDP);
    aiHints.ai_socktype = SOCK_DGRAM;
  }
  aiHints.ai_protocol = protocol;

  int status = socketWrap::getAddrInfo(addr, strPort, &aiHints, &aiList);
  if (status != 0) {
    return INVALID_SOCKET;
  }

  int family = aiList->ai_family;
  wsocket_t sock = socketWrap::socket(family, aiList->ai_socktype, 0);
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

  socketWrap::freeAddrInfo((void *)aiList);
  return sock;

_FAILED:
  socketWrap::close(sock);
_FAILED_FD:
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

int32_t socketSystem::doRequestStart(socketChannel::requestStart *request) {
  int32_t handle = request->_handle;
  socketHandle *s = getSocket(handle);
  if (s->getState() == socketState::INVALID || s->getHandle() != handle) {
    getEventFunc(socketMessageType::M_SOCKET_ERROR)(
        request->_opaque, request->_handle, 0, nullptr, 0);
    return (int32_t)socketMessageType::M_SOCKET_ERROR;
  }

  if (s->getState() == socketState::PACCEPT ||
      s->getState() == socketState::PLISTEN) {
    if (!m_iocp->doRegister(s->getSocket(), (void *)s)) {
      s->getMutexRef()->lock();
      forceClose(s);
      s->getMutexRef()->unlock();
      getEventFunc(socketMessageType::M_SOCKET_ERROR)(
          request->_opaque, request->_handle, 0,
          strerror(errorWrap::wsalasterror()), 0);
      return (int32_t)socketMessageType::M_SOCKET_ERROR;
    }

    s->setState((s->getState() == socketState::PACCEPT) ? socketState::CONNECTED
                                                        : socketState::LISTEN);
    getEventFunc(socketMessageType::M_SOCKET_START)(
        request->_opaque, request->_handle, 0, (void *)"start", 0);
    return -1;
  } else if (s->getState() == socketState::CONNECTED) {
    getEventFunc(socketMessageType::M_SOCKET_START)(
        request->_opaque, request->_handle, 0, (void *)"transfer", 0);
    return -1;
  }
  return -1;
}

int32_t socketSystem::doRequestConnect(socketChannel::requestConnect *request) {
  int32_t handle = request->_handle;
  uintptr_t opaque = request->_opaque;

  char strPort[16];
  char strBuffer[64];
  void *errorPtr = nullptr;
  struct addrinfo aiHints;
  struct addrinfo *aiList = nullptr;
  struct addrinfo *aiPtr = nullptr;

  sprintf(strPort, "%d", request->_port);
  memset(&aiHints, 0, sizeof(aiHints));
  aiHints.ai_family = AF_UNSPEC;
  aiHints.ai_socktype = SOCK_STREAM;
  aiHints.ai_protocol = IPPROTO_TCP;

  wsocket_t sock = INVALID_SOCKET;

  socketHandle *s = getSocket(handle);
  assert(s);

  int status =
      socketWrap::getAddrInfo(request->_addr, strPort, &aiHints, &aiList);
  if (status != 0) {
    errorPtr = (void *)gai_strerror(status);
    goto _FAILED;
  }

  for (aiPtr = aiList; aiPtr != NULL; aiPtr = aiPtr->ai_next) {
    sock = socketWrap::socket(aiPtr->ai_family, aiPtr->ai_socktype,
                              aiPtr->ai_protocol);
    if (sock < 0) {
      continue;
    }

    socketWrap::setKeepalive(sock);
    socketWrap::setNonblock(sock);

    status = socketWrap::connect(sock, aiPtr->ai_addr, aiPtr->ai_addrlen);
    if (status != 0 && errorWrap::wsalasterror() != EINPROGRESS) {
      socketWrap::close(sock);
      sock = INVALID_SOCKET;
      continue;
    }
    break;
  }

  if (sock < 0) {
    errorPtr = (void *)strerror(errorWrap::wsalasterror());
    goto _FAILED;
  }

  if (m_iocp->doRegister(sock, s) != 0) {
    socketWrap::close(sock);
    errorPtr = (void *)"reach network socket number limit";
    goto _FAILED;
  }

  s->doInit(opaque, handle, sock, socketProtocol::TCP);

  if (status == 0) {
    s->setState(socketState::CONNECTED);
    struct sockaddr *addr = aiPtr->ai_addr;
    void *sin_addr = (aiPtr->ai_family == AF_INET)
                         ? (void *)&((struct sockaddr_in *)addr)->sin_addr
                         : (void *)&((struct sockaddr_in6 *)addr)->sin6_addr;
    if (socketWrap::inetNtop(aiPtr->ai_family, sin_addr, strBuffer,
                             sizeof(strBuffer))) {
      errorPtr = strBuffer;
    }

    socketWrap::freeAddrInfo((void *)aiList);
    getEventFunc(socketMessageType::M_SOCKET_START)(
        request->_opaque, request->_handle, 0, errorPtr, 0);
    return -1;
  } else {
    s->setState(socketState::CONNECTING);
    m_iocp->doToWrite(s->getSocket(), s, true);
  }

  socketWrap::freeAddrInfo((void *)aiList);
  return -1;
_FAILED:
  socketWrap::freeAddrInfo((void *)aiList);
  s->setState(socketState::INVALID);
  getEventFunc(socketMessageType::M_SOCKET_ERROR)(
      request->_opaque, request->_handle, 0, errorPtr, 0);
  return (int32_t)socketMessageType::M_SOCKET_ERROR;
}

int32_t socketSystem::doRequestSend(socketChannel::requestSend *request,
                                    const uint8_t *address) {
  int32_t handle = request->_handle;
  socketHandle *s = getSocket(handle);
  /* mutex */
  std::unique_lock<util::spinlock> lock(*s->getMutexRef());

  if (s->getState() == socketState::INVALID ||
      s->getState() == socketState::HALFCLOSE ||
      s->getState() == socketState::PACCEPT || s->getHandle() != handle) {
    util::memory::free(request->_data);
    return -1;
  }

  if (s->getState() == socketState::PLISTEN ||
      s->getState() == socketState::LISTEN) {
    fprintf(stderr, "Socket System: write to listen fd %d.\n", handle);
    util::memory::free(request->_data);
    return -1;
  }

  if (s->isSendQEmpty() && s->getState() == socketState::CONNECTED) {
    s->appendSendQs(request->_data, request->_sz);
    m_iocp->doToWrite(s->getSocket(), (void *)s, true);
  } else {
    s->appendSendQs(request->_data, request->_sz);
  }

  uint32_t warnRet = s->getWarning();
  if (warnRet > 0) {
    getEventFunc(socketMessageType::M_SOCKET_WARNING)(
        s->getOpaque(), request->_handle, warnRet, nullptr, 0);
  }
  return -1;
}

int32_t socketSystem::doRequestSetOpt(socketChannel::requestSetopt *request) {
  int32_t handle = request->_handle;
  socketHandle *s = getSocket(handle);
  if (s->getState() == socketState::INVALID || s->getHandle() != handle) {
    return -1;
  }
  int32_t v = request->_value;
  setsockopt(s->getSocket(), IPPROTO_TCP, request->_what, &v, sizeof(v));
  return -1;
}

int32_t socketSystem::doRequestClose(socketChannel::requestClose *request) {
  int32_t handle = request->_handle;
  socketHandle *s = getSocket(handle);
  if (s->getState() == socketState::INVALID || s->getHandle() != handle) {
    getEventFunc(socketMessageType::M_SOCKET_CLOSE)(
        request->_opaque, request->_handle, 0, nullptr, 0);
    return (int32_t)socketMessageType::M_SOCKET_CLOSE;
  }

  s->getMutexRef()->lock();
  if (s->isSending()) {
    int ret = s->doSend();
    if (ret == (int32_t)socketMessageType::M_SOCKET_CLOSE) {
      forceClose(s);
      s->getMutexRef()->unlock();
      getEventFunc(socketMessageType::M_SOCKET_CLOSE)(
          request->_opaque, request->_handle, 0, nullptr, 0);
      return (int32_t)socketMessageType::M_SOCKET_CLOSE;
    }
  }

  if (request->_shutdown || !s->isSending()) {
    forceClose(s);
    s->getMutexRef()->unlock();
    getEventFunc(socketMessageType::M_SOCKET_CLOSE)(
        request->_opaque, request->_handle, 0, nullptr, 0);
    return (int32_t)socketMessageType::M_SOCKET_CLOSE;
  }
  s->getMutexRef()->unlock();
  s->setState(socketState::HALFCLOSE);
  return -1;
}

void socketSystem::doRequestClearClosedEvent(int32_t handle, int32_t idx,
                                             int32_t n) {
  for (int i = idx; i < n; i++) {
    struct iocpEvent *e = m_iocp->getEvent(i);
    socketHandle *s = static_cast<socketHandle *>(e->s);
    if (s) {
      if (s->getState() == socketState::INVALID && s->getHandle() == handle) {
        e->s = nullptr;
        break;
      }
    }
  }
}

int32_t socketSystem::doConnectProc(socketHandle *s) {
  int error;
  int32_t handle = s->getHandle();
  uintptr_t opaque = s->getOpaque();

  socklen_t len = sizeof(error);
  int code = getsockopt(s->getSocket(), SOL_SOCKET, SO_ERROR, &error, &len);
  if (code < 0 || error) {
    s->getMutexRef()->lock();
    forceClose(s);
    s->getMutexRef()->unlock();

    getEventFunc(socketMessageType::M_SOCKET_ERROR)(
        opaque, handle, 0,
        code >= 0 ? (void *)strerror(errorWrap::wsalasterror())
                  : (void *)strerror(errorWrap::wsalasterror()),
        0);
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
      char buffer[128];
      if (socketWrap::inetNtop(u.s.sa_family, sin_addr, buffer,
                               sizeof(buffer))) {
        getEventFunc(socketMessageType::M_SOCKET_START)(opaque, handle, 0,
                                                        (void *)buffer, 0);
        return -1;
      }
    }
    getEventFunc(socketMessageType::M_SOCKET_START)(opaque, handle, 0, nullptr,
                                                    0);
    return -1;
  }
}

int32_t socketSystem::doAcceptProc(socketHandle *s) {
  union socketAddr u;
  int32_t handle = s->getHandle();
  uintptr_t opaque = s->getOpaque();

  socklen_t len = sizeof(u);
  wsocket_t clientSock = socketWrap::accept(s->getSocket(), &u.s, &len);
  if (clientSock < 0) {
    if (errorWrap::wsalasterror() == EMFILE ||
        errorWrap::wsalasterror() == ENFILE) {
      getEventFunc(socketMessageType::M_SOCKET_ERROR)(
          opaque, handle, 0, (void *)strerror(errorWrap::wsalasterror()), 0);
      return -1;
    } else {
      return 0;
    }
  }

  int32_t clientHandle = getReserve();
  if (clientHandle < 0) {
    socketWrap::close(clientSock);
    return 0;
  }

  socketWrap::setKeepalive(clientSock);
  socketWrap::setNonblock(clientSock);
  socketHandle *cs = getSocket(clientHandle);
  assert(cs->getState() == socketState::RESERVE);
  cs->doInit(s->getOpaque(), clientHandle, clientSock, socketProtocol::TCP);

  void *sin_addr = (u.s.sa_family == AF_INET) ? (void *)&u.v4.sin_addr
                                              : (void *)&u.v6.sin6_addr;
  int sin_port =
      ntohs((u.s.sa_family == AF_INET) ? u.v4.sin_port : u.v6.sin6_port);
  char tmp[INET6_ADDRSTRLEN];
  char buffer[128];
  char *ipAddr = nullptr;
  if (socketWrap::inetNtop(u.s.sa_family, sin_addr, tmp, sizeof(tmp))) {
    snprintf(buffer, sizeof(buffer), "%s:%d", tmp, sin_port);
    ipAddr = buffer;
  }

  getEventFunc(socketMessageType::M_SOCKET_ACCEPT)(opaque, handle, clientHandle,
                                                   ipAddr, 0);
  return 0;
}

int32_t socketSystem::doRecvProc(socketHandle *s) {
  int32_t handle = s->getHandle();
  uintptr_t opaque = s->getOpaque();

  int32_t bufferSize = s->getRecvBufferSize();
  char *buffer = (char *)util::memory::malloc(bufferSize);
  assert(buffer);
  int n = socketWrap::recv(s->getSocket(), buffer, bufferSize);
  if (n < 0) {
    util::memory::free(buffer);
    switch (errorWrap::wsalasterror()) {
    case EINTR:
      break;
    case AGAIN_WOULDBLOCK:
      fprintf(stderr, "Socket System: EAGAIN capture.\n");
      break;
    default:
      s->getMutexRef()->lock();
      forceClose(s);
      s->getMutexRef()->unlock();
      getEventFunc(socketMessageType::M_SOCKET_ERROR)(
          opaque, handle, 0, (void *)strerror(errorWrap::wsalasterror()), 0);
      return (int32_t)socketMessageType::M_SOCKET_ERROR;
    }
    return -1;
  }

  if (n == 0) {
    util::memory::free(buffer);
    s->getMutexRef()->lock();
    forceClose(s);
    s->getMutexRef()->unlock();
    getEventFunc(socketMessageType::M_SOCKET_CLOSE)(opaque, handle, 0, nullptr,
                                                    0);
    return (int32_t)socketMessageType::M_SOCKET_CLOSE;
  }
  if (s->getState() == socketState::HALFCLOSE) {
    util::memory::free(buffer);
    return -1;
  }

  if (n == bufferSize) {
    bufferSize *= 2;
  } else if (bufferSize > SOCKET_HANDLE_MINRECVBUFFER_MAX &&
             n * 2 > bufferSize) {
    bufferSize /= 2;
  }
  s->setRecvBufferSize(bufferSize);
  s->doUpdateRecv(INST(operation::clock, now), n);

  getEventFunc(socketMessageType::M_SOCKET_DATA)(opaque, handle, 0, buffer, n);
  return (int32_t)socketMessageType::M_SOCKET_DATA;
}

int32_t socketSystem::doSendProc(socketHandle *s) {
  if (!s->getMutexRef()->try_lock()) {
    return -1;
  }

  int32_t handle = s->getHandle();
  uintptr_t opaque = s->getOpaque();

  if (s->doSend() == (int32_t)socketMessageType::M_SOCKET_CLOSE) {
    forceClose(s);
    s->getMutexRef()->unlock();
    getEventFunc(socketMessageType::M_SOCKET_CLOSE)(opaque, handle, 0, nullptr,
                                                    0);
    return -1;
  }

  if (s->isSendQEmpty()) {
    m_iocp->doToWrite(s->getSocket(), s, false);
  }

  if (s->getState() == socketState::HALFCLOSE) {
    forceClose(s);
    s->getMutexRef()->unlock();
    getEventFunc(socketMessageType::M_SOCKET_CLOSE)(opaque, handle, 0, nullptr,
                                                    0);
    return (int32_t)socketMessageType::M_SOCKET_CLOSE;
  }

  s->getMutexRef()->unlock();

  return -1;
}

void socketSystem::forceClose(socketHandle *s) {
  if (s->getState() == socketState::INVALID) {
    return;
  }

  assert(s->getState() != socketState::RESERVE);

  if (s->getState() != socketState::PACCEPT &&
      s->getState() != socketState::PLISTEN) {
    m_iocp->doUnRegister(s->getSocket());
  }

  s->doClose();
}

void socketSystem::doPoll() {
  int32_t n, idx = 0;
  for (;;) {

    int r = m_channel->doWait(idx, n);
    if (r == -1) {
      if (m_channel->getError() == socketChannel::errCode::CH_EINTR ||
          m_channel->getError() == socketChannel::errCode::CH_ERROR) {
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
        if (errorWrap::wsalasterror() == EINTR) {
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

    switch (s->getState()) {
    case socketState::CONNECTING:
      doConnectProc(s);
      break;
    case socketState::LISTEN:
      doAcceptProc(s);
      break;
    case socketState::INVALID:
      break;
    default:
      if (e->isread) {
        int32_t type = doRecvProc(s);
        if (e->iswrite && type != (int32_t)socketMessageType::M_SOCKET_CLOSE &&
            type != (int32_t)socketMessageType::M_SOCKET_ERROR) {
          e->isread = false;
          --idx;
        }
        break;
      }
      if (e->iswrite) {
        doSendProc(s);
        break;
      }
      if (e->iserror) {
        int error;
        socklen_t len = sizeof(error);
        int code =
            getsockopt(s->getSocket(), SOL_SOCKET, SO_ERROR, &error, &len);
        const char *err = NULL;
        if (code < 0) {
          err = strerror(errorWrap::wsalasterror());
        } else if (error != 0) {
          err = strerror(error);
        } else {
          err = "Unknown error";
        }

        int32_t handle = s->getHandle();
        uintptr_t opaque = s->getOpaque();

        s->getMutexRef()->lock();
        forceClose(s);
        s->getMutexRef()->unlock();

        getEventFunc(socketMessageType::M_SOCKET_ERROR)(opaque, handle, 0,
                                                        (void *)err, 0);
        break;
      }
      if (e->iseof) {
        int32_t handle = s->getHandle();
        uintptr_t opaque = s->getOpaque();

        s->getMutexRef()->lock();
        forceClose(s);
        s->getMutexRef()->unlock();

        getEventFunc(socketMessageType::M_SOCKET_CLOSE)(opaque, handle, 0,
                                                        nullptr, 0);
      }
      break;
    }
  }
}

NS_CC_N_END