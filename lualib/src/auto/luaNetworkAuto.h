#ifndef LUALIB_NETWORK_AUTO_H
#define LUALIB_NETWORK_AUTO_H

extern "C" {
#include "tolua++.h"
}

#include <api.h>

NS_CC_BEGIN

int regiserAllNetwork(lua_State *l);

NS_CC_END

#endif