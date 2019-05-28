#include "framework.h"
#include "api.h"
#include "util/timestamp.h"
#include <string>
#include <assert.h>

// 临时测试用------------------- begin
#include "test/test_actor.h"
// 临时测试用------------------- end

namespace wolf
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

bool framework::doInit(const commandLineOption *opt)
{
  if (!INST(coroutineOptions,
            load,
            ((commandLineOption *)opt)->getOption("p")))
  {
    return false;
  }

  /*基础配置信息-------------------------------------------------------------------------------------------------------------*/
  INSTGET_VAR(coroutineOptions, _debug) = INST(coroutineOptions, getInt, "debug", 0);
  INSTGET_VAR(coroutineOptions, _thread) = INST(coroutineOptions, getInt, "thread", 6);
  INSTGET_VAR(coroutineOptions, _stackSize) = INST(coroutineOptions, getInt, "stack_size", 1 * 1024 * 1024);
  INSTGET_VAR(coroutineOptions, _single_timeout_us) = INST(coroutineOptions, getInt, "single_timeout", 100 * 1000);
  INSTGET_VAR(coroutineOptions, _dispatcher_thread_interval_us) = INST(coroutineOptions, getInt, "dispatcher_interval", 1000);
  INSTGET_VAR(coroutineOptions, _componentPath) = INST(coroutineOptions, getString, "component_path", "./modules/?.so;./services/?.so");
  INSTGET_VAR(coroutineOptions, _logSize) = INST(coroutineOptions, getInt, "log_size", 64);
  INSTGET_VAR(coroutineOptions, _logLevel) = INST(coroutineOptions, getInt, "log_level", 0);
  INSTGET_VAR(coroutineOptions, _logPath) = INST(coroutineOptions, getString, "log_path", nullptr);
  /*-----------------------------------------------------------------------------------------------------------------------*/

  INST(log::logSystem, doStart);
  INST(operation::scheduler, doStart,
       INSTGET_VAR(coroutineOptions, _thread));
  INST(module::actorSystem, doStart,
       INSTGET_VAR(coroutineOptions, _componentPath));
  INST(network::socketSystem, doStart);

  if (!initialize(opt))
  {
    INST(network::socketSystem, doShutdown);
    INST(module::actorSystem, doShutdown);
    INST(operation::scheduler, doShutdown);
    INST(log::logSystem, doShutdown);
    INST(coroutineOptions, unload);

    return false;
  }

  return true;
}

void framework::doUnInit()
{
  finalize();

  INST(network::socketSystem, doShutdown);
  INST(module::actorSystem, doShutdown);
  INST(operation::scheduler, doShutdown);
  INST(log::logSystem, doShutdown);
  INST(coroutineOptions, unload);
}

} // namespace wolf
