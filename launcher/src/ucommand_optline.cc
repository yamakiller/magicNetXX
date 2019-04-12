#include "ucommand_optline.h"

namespace cis
{
ucommand_optline::~ucommand_optline()
{
    mOptions.clear();
}

bool ucommand_optline::parse(int32_t num, char *args[])
{
    for (int32_t i = 0; i < num; i++)
    {
        std::string arg = std::string(args[i]);
        if (arg.size() < 2 || arg[0] != '-' || arg.find("="))
        {
            continue;
        }

        int npos = arg.find("=");
        std::string key = arg.substr(1, npos - 1);
        std::string value = arg.substr(npos + 1, arg.size() - npos - 1);
        mOptions[key] = value;
    }

    return true;
}

std::string ucommand_optline::getOption(std::string name)
{
    return mOptions[name];
}

bool ucommand_optline::isSet(std::string name)
{
    return mOptions.find(name) != mOptions.end();
}

} // namespace cis