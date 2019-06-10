#ifndef LUALIB_EXTERNAPI_H
#define LUALIB_EXTERNAPI_H

#include <lua.hpp>
#include "luaFix.h"
#if LUA_VERSION_NUM > 501
#define luaL_register(l, name, funs) \
    lua_newtable(l);                 \
    luaL_setfuncs(l, funs, 0);       \
    lua_pushvalue(l, -1);            \
    lua_setglobal(l, name);
#else
#endif

#endif