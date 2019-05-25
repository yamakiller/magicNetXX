#ifndef CIS_ENGINE_NETWORK_SOCKET_HANDLE_H
#define CIS_ENGINE_NETWORK_SOCKET_HANDLE_H

#include "socketWrap.h"
#include "util/spinlock.h"
#include <deque>

#define SOCKET_HANDLE_INVALID -1

namespace engine {
namespace network {

class socketHandle {
  struct sendData {
    void *_data;
    char *_ptr;
    uint32_t _sz;
  };

  struct socketInfo {
    uint64_t _recvLastTime;
    uint64_t _sendLastTime;
    uint64_t _recvBytes;
    uint64_t _sendBytes;
  };

  typedef volatile uint32_t uint32_r;
  typedef std::deque<sendData *> sender_q;

public:
  socketHandle();
  virtual ~socketHandle();

public:
  void doRest();
  void doClose();
  int32_t doSend();

  void doUpdateRecv(uint64_t now, uint64_t bytes);

public:
  bool isSending();
  int32_t incSendIn();
  void decSendIn();

  bool isSendQEmpty() { return m_sendQs.empty(); }

  bool isCanSend(int32_t handle);

  void appendSendQs(const char *data, const int32_t sz);

  void setHandle(int32_t val);
  int32_t getHandle() const;

  void setOpaque(uintptr_t val);
  uintptr_t getOpaque() const;

  void setSocket(wsocket_t val);
  wsocket_t getSocket();

  sendData *getCurrentSendBuffer() { return &m_sendData; }

  void setRecvBufferSize(int32_t val);
  int32_t getRecvBufferSize() { return val; }

  void setProtocol(socketProtocol val);
  socketProtocol getProtocol() const;

  void setState(socketState val);
  bool setCompareState(socketState cval, socketState nval);
  socketState getState() const;

  util::spinlock *getMutexRef();

  socketInfo getInfo() const;

private:
  int32_t m_handle;
  uintptr_t m_opaque;
  wsocket_t m_sock;

private:
  sendData m_curSender;
  uint32_r m_sendIn;
  sender_q m_sendQs;
  uint32_t m_sendBytes;
  uint32_t m_recvBufferSize;

private:
  socketProtocol m_proto;
  socketState m_state;
  socketInfo m_info;

private:
  util::spinlock m_mutex;
};

} // namespace network
} // namespace engine

#endif