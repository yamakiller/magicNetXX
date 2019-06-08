#ifndef LUALIB_STACK_H
#define LUALIB_STACK_H

#include <api.h>
#include <lua.hpp>

NS_CC_BEGIN

class luaStack : public util::mobject {
public:
  luaStack();
  virtual ~luaStack();

public:
  static luaStack *create(module::actor *ptr);

public:
  lua_State *getLuaState(void) { return m_state; }

  virtual void addSearchPath(const char *path);

  virtual void addLoader(lua_CFunction func);

  virtual int reload(const char *moduleFileName);

  virtual int executeScriptFile(const char *filename);

  virtual void clean(void);

  virtual void pushInt(int intValue);

  virtual void pushFloat(float floatValue);

  virtual void pushLong(long longValue);

  virtual void pushBoolean(bool boolValue);

  virtual void pushString(const char *stringValue);

  virtual void pushString(const char *stringValue, int length);

  virtual void pushNil(void);

  virtual int executeFunction(int numArgs);

  int32_t luaLoadBuffer(lua_State *l, const char *chunk, int chunkSize,
                        const char *chunkName);

protected:
  static void *alloc(void *ud, void *ptr, size_t osize, size_t nsize);

protected:
  int32_t init(module::actor *ptr);

protected:
  lua_State *m_state;
  module::actor *m_actor;
  size_t m_mem;
  size_t m_memReport;
  size_t m_memLimit;
};

NS_CC_END

#endif