#include "framework.h"
#include "api.h"
#include "coreDump.h"
#include "util/timestamp.h"
#include <assert.h>
#include <string>


namespace wolf {

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

bool framework::doInit(const commandLineOption *opt) {
  if (!INST(OPT, load, ((commandLineOption *)opt)->getOption("p"))) {
    return false;
  }

  /*基础配置信息-------------------------------------------------------------------------------------------------------------*/
  INSTGET_VAR(OPT, _debug) = INST(OPT, getInt, "debug", 0);
  INSTGET_VAR(OPT, _thread) = INST(OPT, getInt, "thread", 6);
  INSTGET_VAR(OPT, _stackSize) =
      INST(OPT, getInt, "stack_size", 1 * 1024 * 1024);
  INSTGET_VAR(OPT, _actor_gcc_timeout_us) =
      INST(OPT, getInt, "actor_gcc_timeout", 100 * 1000);
  INSTGET_VAR(OPT, _actor_gcc_sleep_us) =
      INST(OPT, getInt, "actor_gcc_sleep", 1000);
  INSTGET_VAR(OPT, _single_timeout_us) =
      INST(OPT, getInt, "single_timeout", 100 * 1000);
  INSTGET_VAR(OPT, _dispatcher_thread_interval_us) =
      INST(OPT, getInt, "dispatcher_interval", 1000);
  INSTGET_VAR(OPT, _componentPath) =
      INST(OPT, getString, "component_path", "./modules/?.so;./services/?.so");
  INSTGET_VAR(OPT, _logSize) = INST(OPT, getInt, "log_size", 64);
  INSTGET_VAR(OPT, _logLevel) = INST(OPT, getInt, "log_level", 0);
  INSTGET_VAR(OPT, _logPath) = INST(OPT, getString, "log_path", nullptr);
  /*-----------------------------------------------------------------------------------------------------------------------*/

  INST(log::logSystem, doStart);
  INST(coreDump, doListen);
  INST(operation::scheduler, doStart, INSTGET_VAR(OPT, _thread));
  INST(module::actorSystem, doStart, INSTGET_VAR(OPT, _componentPath));
  INST(network::socketSystem, doStart);

  if (!initialize(opt)) {
    INST(network::socketSystem, doShutdown);
    INST(module::actorSystem, doShutdown);
    INST(operation::scheduler, doShutdown);
    INST(log::logSystem, doShutdown);
    INST(OPT, unload);

    return false;
  }

  return true;
}

void framework::doUnInit() {
  finalize();

  INST(network::socketSystem, doShutdown);
  INST(module::actorSystem, doShutdown);
  INST(operation::scheduler, doShutdown);
  INST(log::logSystem, doShutdown);
  INST(util::ofile, clear);
  INST(OPT, unload);
}

} // namespace wolf
