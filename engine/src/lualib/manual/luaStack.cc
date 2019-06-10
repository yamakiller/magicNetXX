#include "luaStack.h"
#include "lualib/auto/luaCoreAuto.h"
#include "lualib/auto/luaNetworkAuto.h"
#include "luaLoader.h"
#include "util/ofile.h"

#define LUA_MEMORY_WARNING_REPORT (1024 * 1024 * 32)

NS_CC_LL_BEGIN

void *luaStack::alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
  luaStack *stack = static_cast<luaStack *>(ud);
  size_t mem = stack->m_mem;
  stack->m_mem += nsize;
  if (ptr)
  {
    stack->m_mem -= osize;
  }
  if (stack->m_memLimit != 0 && stack->m_mem > stack->m_memLimit)
  {
    if (ptr == nullptr || nsize > osize)
    {
      stack->m_mem = mem;
      return nullptr;
    }
  }

  if (stack->m_mem > stack->m_memReport)
  {
    stack->m_memReport *= 2;
    SYSLOG_ERROR(0, "lua Stack Memory warning {:02f} M",
                 (float)stack->m_mem / (1024 * 1024));
  }

  return util::memory::lalloc(ptr, osize, nsize);
}

luaStack::luaStack() : m_mem(0), m_memLimit(0)
{
  m_memReport = LUA_MEMORY_WARNING_REPORT;
}

luaStack::~luaStack()
{
  if (m_state)
  {
    lua_close(m_state);
    m_state = nullptr;
  }
}

luaStack *luaStack::create(module::actor *ptr)
{
  luaStack *stack = new luaStack();
  assert(stack);
  stack->init(ptr);
  return stack;
}

int32_t luaStack::init(module::actor *ptr)
{
  m_aptr = ptr;
  m_state = lua_newstate(&luaStack::alloc, this);
  lua_gc(m_state, LUA_GCSTOP, 0);
  luaL_openlibs(m_state);

  // SET FIX data is the
  lua_pushstring(m_state, LUA_STACK_FIX_ACTOR);
  lua_pushlightuserdata(m_state, (void *)this);
  lua_rawset(m_state, LUA_REGISTRYINDEX);

  const luaL_Reg global_functions[] = {
      {nullptr, nullptr}};

  luaL_register(m_state, "_G", global_functions);

  registerAllCore(m_state);
  registerAllNetwork(m_state);

  addLoader(lua_loader);
  return 0;
}

void luaStack::clean() { lua_settop(m_state, 0); }

void luaStack::pushInt(int intValue) { lua_pushinteger(m_state, intValue); }

void luaStack::pushFloat(float floatValue)
{
  lua_pushnumber(m_state, floatValue);
}

void luaStack::pushLong(long longValue) { lua_pushnumber(m_state, longValue); }

void luaStack::pushBoolean(bool boolValue)
{
  lua_pushboolean(m_state, boolValue);
}

void luaStack::pushString(const char *stringValue)
{
  lua_pushstring(m_state, stringValue);
}

void luaStack::pushString(const char *stringValue, int length)
{
  lua_pushlstring(m_state, stringValue, length);
}

void luaStack::pushNil(void) { lua_pushnil(m_state); }

void luaStack::setSearchPath(const char *path)
{
  lua_pushstring(m_state, path);
  lua_setglobal(m_state, "LUA_PATH");
}

void luaStack::setSearchCPath(const char *cpath)
{
  lua_pushstring(m_state, cpath);
  lua_setglobal(m_state, "LUA_CPATH");
}

void luaStack::addLoader(lua_CFunction func)
{
  if (!func)
  {
    return;
  }

  lua_getglobal(m_state, "package");
  lua_getfield(m_state, -1, "searchers");

  lua_pushcfunction(m_state, func);
  for (int i = (int)(lua_rawlen(m_state, -2) + 1); i > 2; --i)
  {
    lua_rawgeti(m_state, -2, i - 1);

    lua_rawseti(m_state, -3, i);
  }
  lua_rawseti(m_state, -2, 2);

  lua_setfield(m_state, -2, "searchers");

  lua_pop(m_state, 1);
}

void luaStack::reload(const char *moduleFileName)
{
  lua_pushstring(m_state, moduleFileName);
  lua_setglobal(m_state, "LUA_PRELOAD");
}

int luaStack::executeString(const char *codes)
{
  luaL_loadstring(m_state, codes);
  return executeFunction(0);
}

int luaStack::executeScriptFile(const char *filename)
{
  assert(filename);

  int nr = 0;
  std::string fullPath = INST(util::ofile, getfullPathForFilename, filename);
  if (!INST(util::ofile, isExist, fullPath))
  {
    SYSLOG_ERROR(m_aptr->handle(), "lua script file {} does not exist", fullPath.c_str());
    assert(false);
  }

  util::Data *pd = INST(util::ofile, getDataFromFile, fullPath);
  if (pd != nullptr && pd->_bytes != nullptr)
  {
    if (luaLoadBuffer(m_state, pd->_bytes, pd->_len, filename) == 0)
    {
      nr = executeFunction(0);
    }
  }
  return nr;
}

int luaStack::executeFunction(int numArgs)
{
  int functionIndex = -(numArgs + 1);
  if (!lua_isfunction(m_state, functionIndex))
  {
    SYSLOG_ERROR(m_aptr->handle(), "value at stack [{}] is not function",
                 functionIndex);
    lua_pop(m_state, numArgs + 1);
    return 0;
  }

  int traceback = 0;
  lua_getglobal(m_state, "__G__TRACKBACK__");
  if (!lua_isfunction(m_state, -1))
  {
    lua_pop(m_state, 1);
  }
  else
  {
    lua_insert(m_state, functionIndex - 1);
    traceback = functionIndex - 1;
  }

  int error = 0;
  error = lua_pcall(m_state, numArgs, 1, traceback);
  if (error)
  {
    if (traceback == 0)
    {
      SYSLOG_ERROR(m_aptr->handle(), "[LUA ERROR] {}",
                   lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
    }
    else
    {
      lua_pop(m_state, 2);
    }
    return 0;
  }

  int ret = 0;
  if (lua_isnumber(m_state, -1))
  {
    ret = (int)lua_tointeger(m_state, -1);
  }
  else if (lua_isboolean(m_state, -1))
  {
    ret = (int)lua_toboolean(m_state, -1);
  }

  lua_pop(m_state, 1);

  if (traceback)
  {
    lua_pop(m_state, 1);
  }

  return ret;
}

int32_t luaStack::luaLoadBuffer(lua_State *l, const char *chunk, int chunkSize,
                                const char *chunkName)
{
  int r = luaL_loadbuffer(l, chunk, chunkSize, chunkName);
  if (r)
  {
    switch (r)
    {
    case LUA_ERRSYNTAX:
      SYSLOG_ERROR("[LUA ERROR] load \"{}\", error: syntax error during "
                   "pre-compilation.",
                   chunkName);
      break;

    case LUA_ERRMEM:
      SYSLOG_ERROR("[LUA ERROR] load \"{}\", error: memory allocation error.",
                   chunkName);
      break;

    case LUA_ERRFILE:
      SYSLOG_ERROR("[LUA ERROR] load \"{}\", error: cannot open/read file.",
                   chunkName);
      break;

    default:
      SYSLOG_ERROR("[LUA ERROR] load \"{}\", error: unknown", chunkName);
    }
  }

  return r;
}

NS_CC_LL_END