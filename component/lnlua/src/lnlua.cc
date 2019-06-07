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

struct lnluaArg
{
    const char *_scriptName;
    void *_arg;
    int _sz;
};

class lnlua : public actorComponent
{
    static void *
    lalloc(void *ud, void *ptr, size_t osize, size_t nsize)
    {
        lnlua *l = static_cast<lnlua *>(ud);
        size_t mem = l->m_mem;
        l->m_mem += nsize;
        if (ptr)
        {
            l->m_mem -= osize;
        }

        if (l->m_memLimit != 0 && l->m_mem > l->m_memLimit)
        {
            if (ptr == NULL || nsize > osize)
            {
                l->m_mem = mem;
                return NULL;
            }
        }
        if (l->m_mem > l->m_memReport)
        {
            l->m_memReport *= 2;
            SYSLOG_ERROR(l->getHandle(), "Memory warning {:02f} M", (float)l->m_mem / (1024 * 1024));
        }

        return memory::lalloc(ptr, osize, nsize);
    }

    static int
    traceback(lua_State *l)
    {
        const char *msg = lua_tostring(l, 1);
        if (msg)
            luaL_traceback(l, l, msg, 1);
        else
        {
            lua_pushliteral(l, "(no error message)");
        }
        return 1;
    }

public:
    lnlua() : m_mem(0),
              m_memLimit(0)
    {
        m_memReport = MEMORY_WARNING_REPORT;
        m_memLimit = 0;
        //TODO 打开tolua基础库
        m_lstate = lua_newstate(&lnlua::lalloc, this);
        assert(m_lstate);
    }

    ~lnlua()
    {
        lua_close(m_lstate);
        m_lstate = nullptr;
    }

protected:
    int32_t onLaunch(void *parm)
    {
        if (parm == nullptr)
        {
            LOCAL_LOG_ERROR("Please enter the script file name");
            return -1;
        }

        lnluaArg *args = static_cast<lnluaArg *>(parm);
        assert(args);

        lua_gc(m_lstate, LUA_GCSTOP, 0);
        luaL_openlibs(m_lstate);
        lua_pushlightuserdata(m_lstate, m_parent);
        lua_setfield(m_lstate, LUA_REGISTRYINDEX, "wolf_actor");
        lua_pop(m_lstate, 1);

        lua_pushcfunction(m_lstate, lnlua::traceback);
        assert(lua_gettop(m_lstate) == 1);

        int r = luaL_loadfile(m_lstate, args->_scriptName);
        if (r != LUA_OK)
        {
            LOCAL_LOG_ERROR("Can't load {} : {}", args->_scriptName, lua_tostring(m_lstate, -1));
            return -1;
        }

        lua_getglobal(m_lstate, "initial");
        lua_pushlstring(m_lstate, (const char *)args->_arg, args->_sz);
        r = lua_pcall(m_lstate, 1, 0, 1);
        if (r != LUA_OK)
        {
            LOCAL_LOG_ERROR("lua loader error : {}", lua_tostring(m_lstate, -1));
            return 1;
        }

        lua_settop(m_lstate, 0);
        if (lua_getfield(m_lstate, LUA_REGISTRYINDEX, "memlimit") == LUA_TNUMBER)
        {
            size_t limit = lua_tointeger(m_lstate, -1);
            m_memLimit = limit;
            LOCAL_LOG_ERROR("Set memory limit to {:2f} M", (float)limit / (1024 * 1024));
            lua_pushnil(m_lstate);
            lua_setfield(m_lstate, LUA_REGISTRYINDEX, "memlimit");
        }
        lua_pop(m_lstate, 1);

        lua_gc(m_lstate, LUA_GCRESTART, 0);

        return 0;
    }

    void unknownDispatch(int32_t msgId, int32_t session, uint32_t source,
                         void *msg, uint32_t sz)
    {
        lua_getglobal(m_lstate, "dispatch");
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

        lua_pop(m_lstate, 1);
    }

private:
    lua_State *m_lstate;
    size_t m_mem;
    size_t m_memReport;
    size_t m_memLimit;
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