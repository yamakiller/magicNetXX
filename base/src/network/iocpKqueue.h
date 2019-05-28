#ifndef CIS_ENGINE_NETWORK_IOCPKQUEUE_H
#define CIS_ENGINE_NETWORK_IOCPKQUEUE_H

#include "iocpWrap.h"

#ifdef UT_PLATFORM_APPLE

namespace wolf {
namespace network {
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
} // namespace network
} // namespace wolf

#endif

#endif