#include "framework.h"
#include "api.h"
#include "timestamp.h"
#include <assert.h>

namespace engine {

static framework *gInstance = nullptr;

framework::framework() { gInstance = this; }

framework::~framework() {}

framework *framework::instance() { return gInstance; }

void framework::startLoop() {
  static int64_t bt = timestamp::getTime();
  int64_t ct = 0;
  while (true) {
    ct = timestamp::getTime();
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

bool framework::doInit() {
  assert(initialize());
  engine::scheduler::instance()->doStart(
      coroutineOptions::instance()->_threadNum);
  return true;
}

void framework::doUnInit() {
  finalize();
  engine::scheduler::instance()->doShutdown();
}
} // namespace engine
