#ifndef LUALIB_LUAFILE_H
#define LUALIB_LUAFILE_H

#include <api.h>
#include <string>
#include <unordered_map>

NS_CC_BEGIN

struct fileData {
  char *bytes;
  ssize_t len;
};

class luaFile : public util::singleton<luaFile> {
public:
  luaFile() = default;
  ~luaFile() = default;

public:
  void clear();
  bool isExist(const std::string &filename);
  fileData *getDataFromFile(const std::string &filename);

protected:
  fileData *getData(const std::string &filename);
  fileData *getUnData(const std::string &filename);

protected:
  std::unordered_map<std::string, fileData> m_data;
  util::spinlock m_lock;
};

NS_CC_END

#endif