#ifndef CIS_ENGINE_UCOMMAND_OPTLINE_H
#define CIS_ENGINE_UCOMMAND_OPTLINE_H

#include <map>
#include <string>
#include <cinttypes>

namespace cis
{
class ucommand_optline
{
public:
    ucommand_optline() = default;
    ~ucommand_optline();

    bool parse(int32_t num, char *args[]);
    std::string getOption(std::string name);
    bool isSet(std::string name);

private:
    std::map<std::string, std::string> mOptions;
};
} // namespace cis

#endif
