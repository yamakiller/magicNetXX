#ifndef WOLF_NETWORK_IOCPWRAP_H
#define WOLF_NETWORK_IOCPWRAP_H

#include "socketWrap.h"
#include <assert.h>

#define IOCP_EVENT_WAIT_MAX 1024

NS_CC_N_BEGIN

struct iocpEvent {
  void *s;
  bool isread;
  bool iswrite;
  bool iserror;
  bool iseof;
};

class iocpWrap {
public:
  virtual ~iocpWrap(){};

  inline struct iocpEvent *getEvent(int32_t idx) {
    assert(idx >= 0 && idx < IOCP_EVENT_WAIT_MAX);
    return &m_evts[idx];
  }

public:
  virtual bool doRegister(wsocket_t sock, void *ud) = 0;
  virtual void doUnRegister(wsocket_t sock) = 0;
  virtual void doToWrite(wsocket_t sock, void *ud, bool enable) = 0;

  virtual int onWait() = 0;

protected:
  struct iocpEvent m_evts[IOCP_EVENT_WAIT_MAX];
};

NS_CC_N_END

#endif