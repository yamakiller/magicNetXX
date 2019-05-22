#ifndef CIS_ENGINE_ICOMMANDLINE_H
#define CIS_ENGINE_ICOMMANDLINE_H

#include <cinttypes>
#include <map>
#include <string>


namespace engine {

class icommandLine {
public:
  virtual bool isSet(const std::string &name) = 0;
  virtual std::string getOption(const std::string &name) = 0;
};

} // namespace engine

#endif