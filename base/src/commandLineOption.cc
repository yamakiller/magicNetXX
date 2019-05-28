#include "commandLineOption.h"

namespace engine {

void commandLineOption::setOption(const std::string &name, bool required) {
  m_required[name] = required;
}

bool commandLineOption::parse(int32_t num, char *args[]) {
  for (int32_t i = 0; i < num; i++) {
    std::string arg = std::string(args[i]);
    if (arg.size() != 2 || arg[0] != '-') {
      continue;
    }
    std::string option = "0";
    option[0] = arg[1];

    m_options[option] = "";
    if (i < num - 1 && !isOption(args[i + 1])) {
      m_options[option] = args[i + 1];
    }
  }

  for (std::map<std::string, bool>::iterator iter = m_required.begin();
       iter != m_required.end(); ++iter) {
    if (iter->second && m_options.find(iter->first) == m_options.end()) {
      fprintf(stderr, "required option -%s not found.", iter->first.c_str());
      return false;
    }
  }

  return true;
}

bool commandLineOption::isSet(const std::string &name) {
  return m_options.find(name) != m_options.end();
}

std::string commandLineOption::getOption(const std::string &name) {
  return m_options[name];
}

bool commandLineOption::isOption(const std::string &val) {
  return val.size() == 2 && val[0] == '-';
}

} // namespace engine