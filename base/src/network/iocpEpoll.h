#ifndef CIS_ENGINE_NETWORK_IOCPEPOLL_H
#define CIS_ENGINE_NETWORK_IOCPEPOLL_H

#include "iocpWrap.h"

#ifdef UT_PLATFORM_LINUX

namespace engine {
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
} // namespace engine

#endif

#endif