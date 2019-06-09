#ifndef LAUNCHER_ENGINE_COMMANDLINEOPTION_H
#define LAUNCHER_ENGINE_COMMANDLINEOPTION_H

#include "platform.h"
#include <map>
#include <string>

NS_CC_BEGIN

class commandLineOption {
public:
  void setOption(const std::string &name, bool required);
  bool parse(int32_t num, char *args[]);
  bool isSet(const std::string &name);
  std::string getOption(const std::string &name);

private:
  bool isOption(const std::string &val);

private:
  std::map<std::string, bool> m_required;
  std::map<std::string, std::string> m_options;
};

NS_CC_END

#endif