#include "stringUtil.h"

namespace engine {
namespace util {

std::vector<std::string> stringUtil::split(const std::string &val,
                                           const std::string &spl) {
  std::vector<std::string> rv;
  std::string::size_type ppos = 0;
  std::string::size_type pos = 0;
  while ((pos = val.find_first_of(spl, ppos)) != std::string::npos) {
    rv.push_back(val.substr(ppos, pos - ppos));
    ++pos;
    ppos = pos;
  }
  rv.push_back(val.substr(ppos, val.size()));
  return rv;
}

std::string stringUtil::replace(const std::string &str_src,
                                const std::string &old_value,
                                const std::string &new_value) {
  std::string str = str_src;
  while (true) {
    std::string::size_type pos(0);
    if ((pos = str.find(old_value)) != std::string::npos)
      str.replace(pos, old_value.length(), new_value);
    else
      break;
  }
  return str;
}

std::string stringUtil::toupper(const std::string &str) {
  std::string res(str);
  std::transform(str.begin(), str.end(), res.begin(), ::toupper);
  return res;
}

std::string stringUtil::tolower(const std::string &str) {

  std::string res(str);
  std::transform(str.begin(), str.end(), res.begin(), ::tolower);
  return res;
}

} // namespace util
} // namespace engine