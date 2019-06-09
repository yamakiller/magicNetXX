#include "luaNetworkAuto.h"

NS_CC_LL_BEGIN

int lua_network_socket_listen(lua_State *l) { return 0; }
int lua_network_socket_open(lua_State *l) { return 0; }
int lua_network_socket_connect(lua_State *l) { return 0; }
int lua_network_socket_close(lua_State *l) { return 0; }

int register_network_socket(lua_State *l)
{
  tolua_usertype(l, "cnet.Socket");
  tolua_cclass(l, "Socket", "cnet.Socket", nullptr, nullptr);
  tolua_beginmodule(l, "Socket");
  tolua_function(l, "listen", lua_network_socket_listen);
  tolua_function(l, "open", lua_network_socket_open);
  tolua_function(l, "connect", lua_network_socket_connect);
  tolua_function(l, "close", lua_network_socket_close);
  tolua_endmodule(l);
  return 0;
}

TOLUA_API int registerAllNetwork(lua_State *l)
{
  tolua_open(l);
  tolua_module(l, "cnet", 0);
  tolua_beginmodule(l, "cnet");

  register_network_socket(l);

  tolua_endmodule(l);
  return 1;
}

NS_CC_LL_END