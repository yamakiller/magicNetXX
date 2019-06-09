#ifndef LUALIB_LOADER_H
#define LUALIB_LOADER_H

#include "base.h"
#include <lua.hpp>

NS_CC_LL_BEGIN

extern "C" int lua_loader(lua_State *l);

NS_CC_LL_END

#endif