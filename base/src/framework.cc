#include "framework.h"
#include "api.h"
#include "util/timestamp.h"
#include <assert.h>

// 临时测试用------------------- begin
#include "test/test_actor.h"
// 临时测试用------------------- end

namespace engine
{

static framework *gInstance = nullptr;

framework::framework() { gInstance = this; }

framework::~framework() {}

framework *framework::instance() { return gInstance; }

void framework::startLoop()
{
  static int64_t bt = util::timestamp::getTime();
  int64_t ct = 0;
  while (true)
  {
    ct = util::timestamp::getTime();
    if (ct <= bt)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    if (!loop())
    {
      break;
    }
    bt = ct;
  }
}

bool framework::doInit(const icommandLine *opt)
{
  INST(log::logSystem, doStart);
  INST(operation::scheduler, doStart,
       INSTGET_VAR(coroutineOptions, _threadNum));
  INST(module::actorSystem, doStart);
  INST(network::socketSystem, doStart);

  REGISTER_SOCKET_MESSAGE(network::socketMessageType::M_SOCKET_ACCEPT, onSocketAccept);
  REGISTER_SOCKET_MESSAGE(network::socketMessageType::M_SOCKET_START, onSocketStart);
  REGISTER_SOCKET_MESSAGE(network::socketMessageType::M_SOCKET_DATA, onSocketData);
  REGISTER_SOCKET_MESSAGE(network::socketMessageType::M_SOCKET_ERROR, onSocketError);
  REGISTER_SOCKET_MESSAGE(network::socketMessageType::M_SOCKET_CLOSE, onSocketClose);
  REGISTER_SOCKET_MESSAGE(network::socketMessageType::M_SOCKET_WARNING, onSocketWarn);

  assert(initialize(opt));

  //临时测试
  test::testActor *test_1 = new test::testActor();
  uint32_t handle = test_1->doInit("testActor");
  INST(module::actorSystem, doSendMessage, 0, handle, module::messageId::M_ID_TEXT);
  INST(module::actorSystem, doSendMessage, 0, handle, module::messageId::M_ID_TEXT);

  return true;
}

void framework::doUnInit()
{
  finalize();

  INST(network::socketSystem, doShutdown);
  INST(module::actorSystem, doShutdown);
  INST(operation::scheduler, doShutdown);
  INST(log::logSystem, doShutdown);
}

void framework::onSocketAccept(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) {}
void framework::onSocketStart(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) {}
void framework::onSocketData(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) {}
void framework::onSocketClose(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) {}
void framework::onSocketError(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) {}
void framework::onSocketWarn(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) {}

} // namespace engine
