#ifndef WOLF_NETWORK_SOCKET_SYSTEM_H
#define WOLF_NETWORK_SOCKET_SYSTEM_H

#include "base.h"
#include "iocpWrap.h"
#include "socketChannel.h"
#include "socketHandle.h"
#include "util/singleton.h"
#include "util/spinlock.h"
#include <thread>

namespace wolf
{
namespace network
{
struct socketMessage
{
  int32_t _id;
  int32_t _type;
  int _ext;
  char *_buffer;
};

class socketSystem : public util::singleton<socketSystem>
{

  friend class socketChannel;
  typedef std::function<void(uintptr_t opaque, int32_t handle, int32_t ud,
                             void *data, size_t sz)>
      socketEventFunc;
  static constexpr int socketEventMax =
      (((int32_t)socketMessageType::M_SOCKET_MAX) - 1);

public:
  socketSystem();
  ~socketSystem();

  int32_t doStart();
  void doShutdown();

  void doRegisterEvent(socketMessageType type, socketEventFunc Func);

  int32_t doSocketListen(uintptr_t opaque, const char *addr, int32_t port,
                         int32_t block = 1024);

  int32_t doSocketConnect(uintptr_t opaque, const char *addr, int32_t port);

  int32_t doSocketStart(uintptr_t opaque, const int32_t handle);

  int32_t doSocketNodelay(int32_t handle);

  int32_t doSocketSend(int32_t handle, const char *data, size_t sz);

  void doSocketClose(uintptr_t opaque, int32_t handle);

  void doSocketShutdown(uintptr_t opaque, int32_t handle);

private:
  wsocket_t doSocketBind(const char *addr, int32_t port, int protocol);

private:
  int32_t getReserve();
  socketHandle *getSocket(int32_t handle);

  int32_t getEventFuncPos(socketMessageType type)
  {
    return ((int32_t)type) - 1;
  }

  socketEventFunc getEventFunc(socketMessageType type)
  {
    return m_eFunc[getEventFuncPos(type)];
  }

  void forceClose(socketHandle *s);

private:
  int32_t doRequestStart(socketChannel::requestStart *request);

  int32_t doRequestConnect(socketChannel::requestConnect *request);

  int32_t doRequestSend(socketChannel::requestSend *request,
                        const uint8_t *address);

  int32_t doRequestSetOpt(socketChannel::requestSetopt *request);

  int32_t doRequestClose(socketChannel::requestClose *request);

  void doRequestClearClosedEvent(int32_t handle, int32_t idx, int32_t n);
  //-----------------------------------------------------
  int32_t doConnectProc(socketHandle *s);

  int32_t doAcceptProc(socketHandle *s);

  int32_t doRecvProc(socketHandle *s);

  int32_t doSendProc(socketHandle *s);

  //------------------------------------------------------
  void doPoll();

private:
  iocpWrap *m_iocp;
  socketChannel *m_channel;
  socketEventFunc m_eFunc[socketEventMax];

private:
  socketHandle m_sockGroup[MAX_SOCKET];
  atomic_t<int32_t> m_sockSequence;

private:
  std::thread m_pid;
};
} // namespace network
} // namespace wolf

#endif