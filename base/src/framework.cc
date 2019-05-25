#include "framework.h"
#include "api.h"
#include "util/timestamp.h"
#include <assert.h>

namespace engine {

static framework *gInstance = nullptr;

framework::framework() { gInstance = this; }

framework::~framework() {}

framework *framework::instance() { return gInstance; }

void framework::startLoop() {
  static int64_t bt = util::timestamp::getTime();
  int64_t ct = 0;
  while (true) {
    ct = util::timestamp::getTime();
    if (ct <= bt) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    if (!loop()) {
      break;
    }
    bt = ct;
  }
}

bool framework::doInit(const icommandLine *opt) {
  INST(log::logSystem, doStart);
  INST(operation::scheduler, doStart,
       INSTGET_VAR(coroutineOptions, _threadNum));
  INST(module::actorSystem, doStart);

  assert(initialize(opt));
  return true;
}

void framework::doUnInit() {
  finalize();

  INST(module::actorSystem, doShutdown);
  INST(operation::scheduler, doShutdown);
  INST(log::logSystem, doShutdown);
}
} // namespace engine
