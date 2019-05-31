#include "api/socket.h"
#include "module/actorComponent.h"

using namespace wolf;
using namespace wolf::module;
using namespace wolf::api;
class SocketTest : public actorComponent {
public:
  SocketTest();
  virtual ~SocketTest();

protected:
  void onLaunch() {
    const char *addr = "127.0.0.1";
    const int port = 29810;
    socketApi::doRequire(this);
    int32_t sock = socketApi::doListen(getActorPtr(), addr, port, 1024);
    if (sock == -1) {
      SYSLOG_ERROR(m_parent->handle(), "监听失败:[{}:{}]", addr, port);
      return;
    }

    SYSLOG_ERROR(m_parent->handle(), "监听成功:[{}:{}]", addr, port);
  }
};