#include "config.h"
#include "util/stringUtil.h"
#include <tinyxml2.h>

using namespace tinyxml2;

NS_CC_BEGIN

bool coroutineOptions::load(std::string path)
{
    if (path.empty() || path.compare("") == 0)
    {
        fprintf(stderr, "Please enter the configuration file path, the command parameters are:-p <filepath>");
        return false;
    }

    XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
    {
        fprintf(stderr, "Profile open failed:%s", path.c_str());
        return false;
    }

    XMLElement *root = doc.RootElement();
    XMLElement *options = root->FirstChildElement("options");

    if (options)
    {
        const XMLElement *itemPtr = options->FirstChildElement();
        while (itemPtr)
        {
            m_env[itemPtr->Name()] = itemPtr->GetText();
            itemPtr = itemPtr->NextSiblingElement();
        }
    }

    return true;
}

void coroutineOptions::unload()
{
    m_env.clear();
}

int coroutineOptions::getInt(std::string const &key, int defval)
{
    const char *value = getValue(key);
    if (value == nullptr)
    {
        return defval;
    }
    return atoi(value);
}

bool coroutineOptions::getBool(std::string const &key, bool defval)
{
    const char *value = getValue(key);
    if (value == nullptr)
    {
        return defval;
    }
    std::string tmpstr = std::string(value);
    tmpstr = util::stringUtil::tolower(tmpstr);
    if (tmpstr.compare("false") == 0 ||
        tmpstr.compare("0") == 0)
    {
        return false;
    }
    return true;
}

const char *coroutineOptions::getString(std::string const &key, const char *defval)
{
    const char *value = getValue(key);
    if (value == nullptr)
    {
        return defval;
    }
    return value;
}

const char *coroutineOptions::getValue(std::string const &key)
{
    if (m_env.empty())
    {
        return nullptr;
    }

    auto it = m_env.find(key);
    if (it == m_env.end())
    {
        return nullptr;
    }

    if (it->second.empty() && it->second.compare("") == 0)
    {
        return nullptr;
    }

    return it->second.c_str();
}

NS_CC_END
