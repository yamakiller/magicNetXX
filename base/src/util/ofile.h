#ifndef WOLF_UTIL_OFILE_H
#define WOLF_UTIL_OFILE_H

#include <api.h>
#include <string>
#include <unordered_map>

NS_CC_BEGIN

NS_UTIL_BEGIN

struct Data {
  char *_bytes;
  ssize_t _len;
};

class ofile : public util::singleton<ofile> {
public:
  ofile() = default;
  ~ofile() = default;

public:
  void clear();
  bool isExist(const std::string &filename);
  Data *getDataFromFile(const std::string &filename);

protected:
  Data *getData(const std::string &filename);
  Data *getUnData(const std::string &filename);

protected:
  std::unordered_map<std::string, Data> m_data;
  util::spinlock m_lock;
};
NS_UTIL_END

NS_CC_END

#endif