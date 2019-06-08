#include "luaLoader.h"
#include "luaFix.h"
#include "luaStack.h"
#include "util/ofile.h"
#include <string>

using namespace wolf;

extern "C" int lua_loader(lua_State *l) {
  static const std::string BYTECODE_FILE_EXT = ".luac";
  static const std::string NOT_BYTECODE_FILE_EXT = ".lua";

  std::string filename(luaL_checkstring(l, -1));

  size_t pos = filename.rfind(BYTECODE_FILE_EXT);
  if (pos != std::string::npos) {
    filename = filename.substr(0, pos);
  } else {
    pos = filename.rfind(NOT_BYTECODE_FILE_EXT);
    if (pos == filename.length() - NOT_BYTECODE_FILE_EXT.length()) {
      filename = filename.substr(0, pos);
    }
  }

  pos = filename.find_first_of(".");
  while (pos != std::string::npos) {
    filename.replace(pos, 1, "/");
    pos = filename.find_first_of(".");
  }

  util::Data *chunk;
  std::string chunkName;

  lua_getglobal(l, "package");
  lua_getfield(l, -1, "path");
  std::string searchpath(lua_tostring(l, -1));
  lua_pop(l, 1);
  size_t begin = 0;
  size_t next = searchpath.find_first_of(";", 0);

  do {
    if (next == std::string::npos) {
      next = searchpath.length();
    }

    std::string prefix = searchpath.substr(begin, next);
    if (prefix[0] == '.' && prefix[1] == '/') {
      prefix = prefix.substr(2);
    }

    pos = prefix.find("?.lua");
    chunkName = prefix.substr(0, pos) + filename + BYTECODE_FILE_EXT;
    if (INST(util::ofile, isExist, chunkName.c_str())) {
      chunk = INST(util::ofile, getDataFromFile, chunkName.c_str());
      break;
    } else {
      chunkName = prefix.substr(0, pos) + filename + NOT_BYTECODE_FILE_EXT;
      if (INST(util::ofile, isExist, chunkName.c_str())) {
        chunk = INST(util::ofile, getDataFromFile, chunkName.c_str());
        break;
      }
    }

    begin = next + 1;
    next = searchpath.find_first_of(";", begin);

  } while (begin < (int)searchpath.length());

  if (chunk && chunk->_len > 0) {
    lua_getfield(l, LUA_REGISTRYINDEX, LUA_STACK_FIX_ACTOR);
    luaStack *stack = (luaStack *)lua_touserdata(l, -1);
    lua_pop(l, 1);
    stack->luaLoadBuffer(l, chunk->_bytes, chunk->_len, chunkName.c_str());
  } else {
    SYSLOG_ERROR(0, "can not get file data of {}", chunkName.c_str());
    return -1;
  }

  return 0;
}
