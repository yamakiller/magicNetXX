#ifndef WOLF_UTIL_MSTRING_H
#define WOLF_UTIL_MSTRING_H

#include "base.h"
#include "memory.h"
#include <algorithm>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

namespace wolf
{
namespace util
{
class stringUtil : public noncopyable
{
public:
  inline static char *strdup(const char *str)
  {
    size_t strsz = strlen(str);
    char *dstr = (char *)memory::malloc(strsz + 1);
    memcpy(dstr, str, strsz);
    dstr[strsz] = '\0';
    return dstr;
  }

  static std::vector<std::string> split(const std::string &val,
                                        const std::string &spl);

  // 字符串替换
  static std::string replace(const std::string &str_src,
                             const std::string &old_value,
                             const std::string &new_value);
  // 转换为大写
  static std::string toupper(const std::string &str);
  // 转换为小写
  static std::string tolower(const std::string &str);

  template <typename Container>
  static std::string connect(Container value, const std::string &delim)
  {
    if (value.begin() == value.end())
    {
      return std::string();
    }

    std::stringstream ss;
    typename Container::iterator iter = value.begin();
    ss << *iter;
    ++iter;
    for (; iter != value.end(); ++iter)
    {
      ss << delim << *iter;
    }
    return ss.str();
  }
  template <typename Container>
  static std::string connect(Container value, const std::string &delim,
                             const std::string &before,
                             const std::string &after)
  {
    if (value.begin() == value.end())
    {
      return std::string();
    }

    std::stringstream ss;
    typename Container::iterator iter = value.begin();
    ss << before << *iter << after;
    ++iter;
    for (; iter != value.end(); ++iter)
    {
      ss << delim << before << *iter << after;
    }
    return ss.str();
  }
};
} // namespace util
} // namespace wolf

#endif