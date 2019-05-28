#ifndef WOLF_SOCKET_CHANNEL_H
#define WOLF_SOCKET_CHANNEL_H

#include "iocpWrap.h"
#include <functional>
#include <sys/select.h>
#include <sys/time.h>

#define CHANNEL_INVALID -1

namespace wolf
{
namespace network
{
class socketChannel final
{
public:
  struct requestExit
  {
    uintptr_t _opaque;
  };

  struct requestStart
  {
    int32_t _handle;
    uintptr_t _opaque;
  };

  struct requestConnect
  {
    int32_t _handle;
    int32_t _port;
    uintptr_t _opaque;
    char _addr[1];
  };

  struct requestSend
  {
    int32_t _handle;
    int32_t _sz;
    char *_data;
  };

  struct requestSetopt
  {
    int32_t _handle;
    int32_t _what;
    int32_t _value;
  };

  struct requestClose
  {
    int32_t _handle;
    int32_t _shutdown;
    uintptr_t _opaque;
  };

  struct requestPacket
  {
    uint8_t header[2];
    union {
      char data[256];
      struct requestStart start;
      struct requestSend send;
      struct requestSetopt setopt;
      struct requestClose close;
      struct requestConnect connect;
      struct requestExit exit;
    } u;
    char dummy[256];
  };

  typedef std::function<int32_t(requestStart *)> requestStartFunc;
  typedef std::function<int32_t(requestConnect *)> requestConnectFunc;
  typedef std::function<int32_t(requestSend *, const uint8_t *)>
      requestSendFunc;
  typedef std::function<int32_t(requestSetopt *)> requestSetoptFunc;
  typedef std::function<int32_t(requestClose *)> requestCloseFunc;
  typedef std::function<void(int32_t handle, int32_t idx, int32_t n)>
      requestClearClosedFunc;

public:
  enum errCode
  {
    CH_NOEINTR = 1,
    CH_EINTR = 2,
    CH_ERROR = 3,
    CH_EXIT = 4,
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

  int doWait(int32_t idx, int32_t n);

public:
  void doRegisterStartFunc(requestStartFunc func) { m_startFunc = func; }
  void doRegisterConnectFunc(requestConnectFunc func) { m_connectFunc = func; }
  void doRegisterSendFunc(requestSendFunc func) { m_sendFunc = func; }
  void doRegisterSetOptFunc(requestSetoptFunc func) { m_setoptFunc = func; }
  void doRegisterCloseFunc(requestCloseFunc func) { m_closeFunc = func; }
  void doRegisterClearClosedFunc(requestClearClosedFunc func)
  {
    m_clearClosedFunc = func;
  }

private:
  void doCloseCtrl(int *ctrl);
  void doRecv(void *buffer, const size_t bytes);
  void doWrite(const char *data, const size_t bytes);

  int doProccess(int32_t idx, int32_t n);
  bool isRecv();

private:
  requestStartFunc m_startFunc;
  requestConnectFunc m_connectFunc;
  requestSendFunc m_sendFunc;
  requestSetoptFunc m_setoptFunc;
  requestCloseFunc m_closeFunc;
  requestClearClosedFunc m_clearClosedFunc;

private:
  int m_recvCtrl;
  int m_sendCtrl;
  int m_controlCtrl;
  int m_error;

private:
  fd_set m_rsfds;
};
} // namespace network
} // namespace wolf
#endif