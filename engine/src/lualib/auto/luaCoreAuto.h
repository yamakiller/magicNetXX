#ifndef LUALIB_CORE_AUTO_H
#define LUALIB_CORE_AUTO_H

extern "C"
{
#include "tolua++.h"
}

#include "base.h"

NS_CC_LL_BEGIN

int registerAllCore(lua_State *l);

NS_CC_LL_END

#endif