#include "luaCoreAuto.h"
#include "lualib/manual/luaStack.h"
#include "module/actor.h"

NS_CC_LL_BEGIN

module::actor *lua_core_get_actor(lua_State *LS) {
  module::actor *cobj = nullptr;
  lua_getfield(LS, LUA_REGISTRYINDEX, LUA_STACK_FIX_ACTOR);
  luaStack *stack = (luaStack *)lua_touserdata(LS, -1);
  cobj = stack->getActor();
  lua_pop(LS, 1);
  return cobj;
}

int lua_core_actor_create(lua_State *LS) {
  int argc = 0;

#if WOLF_DEBUG >= 1
  tolua_Error tolua_err;
  if (!tolua_isusertable(LS, 1, "ccore.Actor", 0, &tolua_err)) {
    goto tolua_lerror;
  }
#endif

  argc = lua_gettop(LS) - 1;
  if (argc == 2) {
    module::actor *ret = new module::actor();
    uint32_t handle =
        ret->doInit(luaL_checkstring(LS, 2), (void *)lua_tostring(LS, 3));
    if (handle == 0) {
      return 0;
    }
    lua_pushnumber(LS, handle);
    return 1;
  }

  luaL_error(LS, "%s has wrong number of arguments: %d, was expecting %d \n",
             "ccore.Actor:create", argc, 2);
  return 0;

#if WOLF_DEBUG >= 1
tolua_lerror:
  tolua_error(LS, "#ferror in function 'lua_core_actor_create'.", &tolua_err);
#endif

  return 0;
}

int lua_core_actor_callback(lua_State *LS) {
  int argc;
  module::actor *cobj = nullptr;
#if WOLF_DEBUG >= 1
  tolua_Error tolua_err;
#endif

#if WOLF_DEBUG >= 1
  if (!tolua_isusertype(LS, 1, "ccore.Actor", 0, &tolua_err))
    goto tolua_lerror;
#endif

  argc = lua_gettop(LS) - 1;
  if (argc == 1) {
    luaL_checktype(LS, 2, LUA_TFUNCTION);
    lua_pushstring(LS, LUA_STACK_FIX_CALLBACK);
    lua_settop(LS, 2);
    lua_rawset(LS, LUA_REGISTRYINDEX);
    return 0;
  }

  luaL_error(LS, "%s has wrong number of arguments: %d, was expecting %d \n",
             "ccore.Actor:callback", argc, 1);
  return 0;

#if WOLF_DEBUG >= 1
tolua_lerror:
  tolua_error(LS, "#ferror in function 'lua_core_actor_init'.", &tolua_err);
#endif
  return 0;
}

int lua_core_actor_send(lua_State *LS) { return 0; }

int lua_core_actor_timeout(lua_State *LS) { return 0; }

int lua_core_actor_susped(lua_State *LS) { return 0; }

int lua_core_actor_wakeup(lua_State *LS) { return 0; }

int lua_core_actor_logout(lua_State *LS) {
  int argc;
  module::actor *cobj = nullptr;
#if WOLF_DEBUG >= 1
  tolua_Error tolua_err;
#endif

#if WOLF_DEBUG >= 1
  if (!tolua_isusertype(LS, 1, "ccore.Actor", 0, &tolua_err)) {
    goto tolua_lerror;
  }
#endif

  cobj = lua_core_get_actor(LS);

  argc = lua_gettop(LS) - 1;
  if (argc == 2) {
    int type = tolua_tonumber(LS, 3, 0);
    switch (type) {
    case log::ilog::L_INFO:
      SYSLOG_INFO(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    case log::ilog::L_ERROR:
      SYSLOG_ERROR(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    case log::ilog::L_TRACE:
      SYSLOG_TRACE(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    case log::ilog::L_WARNING:
      SYSLOG_WARNING(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    case log::ilog::L_DEBUG:
      SYSLOG_DEBUG(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    case log::ilog::L_FATAL:
      SYSLOG_FATAL(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    default:
      SYSLOG_INFO(cobj->handle(), tolua_tostring(LS, 2, ""));
      break;
    }

    return 0;
  }

  luaL_error(LS, "%s has wrong number of arguments: %d, was expecting %d\n ",
             "ccore.Actor:logout", argc, 2);
  return 0;
#if WOLF_DEBUG >= 1
tolua_lerror:
  tolua_error(LS, "#ferror in function 'lua_core_actor_trace'.", &tolua_err);
#endif
  return 0;
}

int lua_core_actor_exit(lua_State *LS) {
  int argc;
  module::actor *cobj = nullptr;
#if WOLF_DEBUG >= 1
  tolua_Error tolua_err;
#endif

#if WOLF_DEBUG >= 1
  if (!tolua_isusertype(LS, 1, "ccore.Actor", 0, &tolua_err))
    goto tolua_lerror;
#endif

  cobj = lua_core_get_actor(LS);

  argc = lua_gettop(LS) - 1;
  if (argc == 0) {
    cobj->doExit();
    return 0;
  }

  luaL_error(LS, "%s has wrong number of arguments: %d, was expecting %d\n ",
             "ccore.Actor:exit", argc, 0);
  return 0;
#if WOLF_DEBUG >= 1
tolua_lerror:
  tolua_error(LS, "#ferror in function 'lua_core_actor_exit'.", &tolua_err);
#endif
  return 0;
}

int register_core_actor(lua_State *l) {
  tolua_usertype(l, "ccore.Actor");
  tolua_cclass(l, "Actor", "ccore.Actor", "", nullptr);
  tolua_beginmodule(l, "Actor");
  tolua_function(l, "new", lua_core_actor_create);
  tolua_function(l, "callback", lua_core_actor_callback);
  tolua_function(l, "send", lua_core_actor_send);
  tolua_function(l, "logout", lua_core_actor_logout);
  tolua_function(l, "timeout", lua_core_actor_timeout);
  tolua_function(l, "susped", lua_core_actor_susped);
  tolua_function(l, "wakeup", lua_core_actor_wakeup);
  tolua_function(l, "exit", lua_core_actor_exit);
  tolua_endmodule(l);
}

TOLUA_API int registerAllCore(lua_State *l) {
  tolua_open(l);
  tolua_module(l, "ccore", 0);
  tolua_beginmodule(l, "ccore");

  register_core_actor(l);

  tolua_endmodule(l);
  return 1;
}

NS_CC_LL_END
