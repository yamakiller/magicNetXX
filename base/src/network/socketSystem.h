#ifndef CIS_ENGINE_NETWORK_SOCKET_SYSTEM_H
#define CIS_ENGINE_NETWORK_SOCKET_SYSTEM_H

#include "base.h"
#include "iocpWrap.h"
#include "socketChannel.h"
#include "socketHandle.h"
#include "util/singleton.h"
#include "util/spinlock.h"
#include <thread>

namespace engine {
namespace network {

class socketSystem : public singleton<socketSystem> {

  friend class socketChannel;

public:
  socketSystem();
  ~socketSystem();

  int32_t doStart();
  void doShutdown();

  int32_t doSocketListen(uintptr_t opaque, const char *addr, int32_t port,
                         int32_t block = 1024);

  int32_t doSocketOpen(const int32_t handle);

  int32_t doSocketSend(int32_t handle, const char *data, size_t sz);

  void doSocketClose(uintptr_t opaque, int32_t handle);

  void doSocketShutdown(uintptr_t opaque, int32_t handle);
  // int32_t doConnect(uintptr_t opaque, const char *addr, int16_t port);
  // int32_t doSend(int32_t handle, const char *data, size_t sz);
  // int32_t doClose(int32_t handle);
private:
  wsocket_t doSocketBind(const char *addr, int32_t port, int protocol);

private:
  int32_t getReserve();
  socketHandle *getSocket(int32_t handle);

private:
  int32_t doSendProccess(socketChannel::requestSend *request,
                         const uint8_t address);
  int32_t doCloseProccess(socketChannel::requestClose *request);

  int32_t doConnectProccess(socketHandle *s);

  int32_t doAcceptProccess(socketHandle *s);

  int32_t doRecvProccess(socketHandle *s);

  int32_t doSendProccess(socketHandle *s);

  //------------------------------------------------------
  void doPoll();

private:
  iocpWrap *m_iocp;
  socketChannel *m_channel;

private:
  socketHandle m_sockGroup[MAX_SOCKET];
  atomic_t<int32_t> m_sockSequence;

private:
  bool m_shutdown;
  std::thread m_pid;
};
} // namespace network
} // namespace engine

#endif