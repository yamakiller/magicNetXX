#ifndef LUALIB_CORE_AUTO_H
#define LUALIB_CORE_AUTO_H

extern "C" {
#include "tolua++.h"
}

#include <api.h>

NS_CC_BEGIN

int regiserAllCore(lua_State *l);

NS_CC_END

#endif