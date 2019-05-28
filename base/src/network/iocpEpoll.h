#ifndef WOLF_NETWORK_IOCPEPOLL_H
#define WOLF_NETWORK_IOCPEPOLL_H

#include "iocpWrap.h"

#ifdef UT_PLATFORM_LINUX

namespace wolf {
namespace network {
class iocpEpoll : public iocpWrap {
public:
  iocpEpoll();
  ~iocpEpoll();

public:
  bool doRegister(wsocket_t sock, void *ud);
  void doUnRegister(wsocket_t sock);
  void doToWrite(wsocket_t sock, void *ud, bool enable);

  int onWait();

protected:
  int32_t m_handle;
};
} // namespace network
} // namespace wolf

#endif

#endif