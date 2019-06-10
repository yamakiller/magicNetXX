#ifndef WOLF_LNLUA_H
#define WOLF_LNLUA_H

#include "api.h"
#include <boost/any.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include <lua.hpp>

#define MEMORY_WARNING_REPORT (1024 * 1024 * 32)

using namespace wolf;
using namespace wolf::module;
using namespace wolf::api;
using namespace wolf::util;
using namespace wolf::lualib;

class lnlua : public actorComponent
{

public:
    lnlua()
    {
    }

    ~lnlua()
    {
    }

protected:
    int32_t onLaunch(void *parm)
    {
        if (parm == nullptr)
        {
            LOCAL_LOG_ERROR("lnlua component error: please input args[<xxx.lua> <args>]");
            return -1;
        }

        m_stack = luaStack::create(m_parent);
        assert(m_stack);
        m_stack->setSearchPath(INST(OPT, getString, "lua_path", "./lua-script/?.lua"));
        m_stack->setSearchCPath(INST(OPT, getString, "lua_cpath", "./lua-lib/?.so"));
        m_stack->reload(INST(OPT, getString, "lua_reload", nullptr));

        std::string strParam = (const char *)parm;
        std::vector<std::string> args = util::stringUtil::split(strParam, " ");
        if (args[0].find_first_of(".lua", 0) == -1)
        {
            args[0] += ".lua";
        }

        int r = m_stack->executeScriptFile(args[0].c_str());
        if (r != LUA_OK)
        {
            LOCAL_LOG_ERROR("can't load {} : {}", args[0].c_str(), lua_tostring(m_stack->getLuaState(), -1));
            return -1;
        }
        return 0;
    }

    void unknownDispatch(int32_t msgId, int32_t session, uint32_t source,
                         void *msg, uint32_t sz)
    {
        /*lua_getglobal(m_lstate, "dispatch");
        lua_pushinteger(m_lstate, msgId);
        lua_pushinteger(m_lstate, session);
        lua_pushinteger(m_lstate, source);
        lua_pushlightuserdata(m_lstate, msg);
        lua_pushinteger(m_lstate, sz);
        int r = lua_pcall(m_lstate, 5, 0, 1);
        switch (r)
        {
        case LUA_ERRRUN:
            LOCAL_LOG_ERROR("lua call [{:08x} to {:08x} : {} message size = {}] error: {}", source, getHandle(), session, sz, lua_tostring(m_lstate, -1));
            break;
        case LUA_ERRMEM:
            LOCAL_LOG_ERROR("lua memory error :[{:x} to {:08x} : {}]", source, getHandle(), session);
            break;
        case LUA_ERRERR:
            LOCAL_LOG_ERROR("lua error in error : [{:x} to {:08x} : {}]", source, getHandle(), session);
            break;
        case LUA_ERRGCMM:
            LOCAL_LOG_ERROR("lua gcc error : [{:x} to {:08x} : {}]", source, getHandle(), session);
            break;
        };

        lua_pop(m_lstate, 1);*/
    }

private:
    luaStack *m_stack;
};

WOLF_C_API void *lnlua_create()
{
    lnlua *ptr = new lnlua();
    assert(ptr);
    return (void *)ptr;
}

WOLF_C_API void lnlua_signal(void *inst, int signal) {}

WOLF_C_API void lnlua_release(void *inst)
{
    lnlua *ptr = (lnlua *)inst;
    delete ptr;
}

#endif