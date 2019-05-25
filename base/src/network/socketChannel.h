#ifndef CIS_ENGINE_SOCKET_CHANNEL_H
#define CIS_ENGINE_SOCKET_CHANNEL_H

#include "iocpWrap.h"
#include <functional>
#include <sys/select.h>
#include <sys/time.h>

#define CHANNEL_INVALID = -1

namespace engine {
namespace network {
class socketChannel final {
public:
  struct requestSend {
    int32_t _handle;
    int32_t _sz;
    char *_data;
  };

  struct requestClose {
    int32_t _handle;
    int32_t _shutdown;
    uintptr_t _opaque;
  };

  struct requestPacket {
    uint8_t header[2];
    union {
      char data[256];
      struct requestSend send;
      struct requestClose close;
    } u;
    char dummy[256];
  };

  typedef std::function<int32_t(requestSend *, const uint8_t *)>
      requestSendFunc;
  typedef std::function<int32_t(requestClose *)> requestCloseFunc;

public:
  enum errCode {
    CH_ERROR = 1,
    CH_EINTR = 2,
    CH_EXIT = 3,
  };

public:
  socketChannel();
  ~socketChannel();

public:
  bool isInvalid();

  int getError();

  void setError(int val);

  void doSend(struct requestPacket *request, char type, int len);

  void doRest();

  int doWait();

public:
  void doRegisterSendFunc(requestSendFunc func) { m_sendFunc = func; }
  void doRegisterCloseFunc(requestCloseFunc func) { m_closeFunc = func; }

private:
  void doCloseCtrl(int *ctrl);
  void doRecv(void *buffer, const size_t bytes);
  void doSend(const char *data, const size_t bytes);

  int doProccess();
  bool isRecv();

private:
  requestSendFunc m_sendFunc;
  requestCloseFunc m_closeFunc;

private:
  int m_recvCtrl;
  int m_sendCtrl;
  int m_controlCtrl;
  int m_error;

private:
  fd_set m_rsfds;
}
} // namespace network
} // namespace engine
#endif