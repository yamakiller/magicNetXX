#ifndef CIS_ENGINE_API_H
#define CIS_ENGINE_API_H

#include "config.h"
#include "log/logSystem.h"
#include "module/actorSystem.h"
#include "operation/scheduler.h"
#include "network/socketSystem.h"
#include "util/stringUtil.h"

#define REGISTER_SOCKET_MESSAGE(type, func) INST(network::socketSystem, doRegisterEvent,                                   \
                                                 type,                                                                     \
                                                 [](uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz) { \
                                                     if (!framework::instance())                                           \
                                                     {                                                                     \
                                                         return;                                                           \
                                                     }                                                                     \
                                                     framework::instance()->func(opaque, handle, ud, data, sz);            \
                                                 })

#endif