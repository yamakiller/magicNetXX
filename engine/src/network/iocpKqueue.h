#ifndef WOLF_NETWORK_IOCPKQUEUE_H
#define WOLF_NETWORK_IOCPKQUEUE_H

#include "iocpWrap.h"

#ifdef UT_PLATFORM_APPLE

NS_CC_N_BEGIN

class iocpKqueue : public iocpWrap {
public:
  iocpKqueue();
  ~iocpKqueue();

public:
  bool doRegister(wsocket_t sock, void *ud);
  void doUnRegister(wsocket_t sock);
  void doToWrite(wsocket_t sock, void *ud, bool enable);

  int onWait();

protected:
  int32_t m_handle;
};

NS_CC_N_END;

#endif

#endif