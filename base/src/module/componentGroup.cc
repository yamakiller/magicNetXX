#include "componentGroup.h"
#include "util/stringUtil.h"

#include <dlfcn.h>

namespace wolf
{
namespace module
{
componentGroup::componentGroup() : m_count(0)
{
    memset(&m_compts, 0, sizeof(struct component) * MAX_COMPONENT_TYPE);
}

componentGroup::~componentGroup()
{
}

int32_t componentGroup::doInit(const char *path)
{
    m_path = std::string(path);
    return 0;
}

void componentGroup::doDestory()
{
    m_mutex.lock();
    for (int i = 0; i < MAX_COMPONENT_TYPE; i++)
    {
        if (m_compts[i]._name)
        {
            util::memory::free((void *)m_compts[i]._name);
        }

        if (m_compts[i]._dll)
        {
            dlclose(m_compts[i]._dll);
        }
    }
    memset(&m_compts, 0, sizeof(struct component) * MAX_COMPONENT_TYPE);
    m_mutex.unlock();
}

struct component *componentGroup::getComponent(const char *name)
{
    struct component *result = get(name);
    if (result)
    {
        return result;
    }

    m_mutex.lock();
    result = get(name);
    if (result == nullptr && m_count < MAX_COMPONENT_TYPE)
    {
        int index = m_count;
        void *dl = tryOpen(name);
        if (dl)
        {
            m_compts[index]._name = name;
            m_compts[index]._dll = dl;

            if (openSym(&m_compts[index]) == 0)
            {
                m_compts[index]._name = util::stringUtil::strdup(name);
                m_count++;
                result = &m_compts[index];
            }
        }
    }

    m_mutex.unlock();

    return result;
}

struct component *componentGroup::get(const char *name)
{
    for (int i = 0; i < m_count; i++)
    {
        if (m_compts[i]._name == nullptr)
        {
            continue;
        }

        if (strcmp(m_compts[i]._name, name) == 0)
        {
            return &m_compts[i];
        }
    }
    return NULL;
}

void *componentGroup::tryOpen(const char *name)
{
    const char *l;
    const char *path = m_path.c_str();
    size_t path_size = strlen(path);
    size_t name_size = strlen(name);

    int sz = path_size + name_size;
    //search path
    void *dl = NULL;
    char tmp[sz];
    do
    {
        memset(tmp, 0, sz);
        while (*path == ';')
            path++;
        if (*path == '\0')
            break;
        l = strchr(path, ';');
        if (l == NULL)
            l = path + strlen(path);
        int len = l - path;
        int i;
        for (i = 0; path[i] != '?' && i < len; i++)
        {
            tmp[i] = path[i];
        }
        memcpy(tmp + i, name, name_size);
        if (path[i] == '?')
        {
            strncpy(tmp + i + name_size, path + i + 1, len - i - 1);
        }
        else
        {
            fprintf(stderr, "Invalid C/CXX service path\n");
            exit(1);
        }
        dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
        path = l;
    } while (dl == NULL);

    if (dl == NULL)
    {
        SYSLOG_ERROR(0, "try open {} failed : {}", name, dlerror());
    }

    return dl;
}

bool componentGroup::openSym(struct component *comp)
{
    comp->_create = (component_dl_create)getApi(comp, "_create");
    comp->_release = (component_dl_release)getApi(comp, "_release");
    comp->_signal = (component_dl_signal)getApi(comp, "_signal");

    return comp->_create != nullptr && comp->_release != nullptr;
}

void *componentGroup::getApi(struct component *comp, const char *apiName)
{
    size_t nameSize = strlen(comp->_name);
    size_t apiSize = strlen(apiName);
    char tmp[nameSize + apiSize + 1];
    memcpy(tmp, comp->_name, nameSize);
    memcpy(tmp + nameSize, apiName, apiSize + 1);
    char *ptr = strrchr(tmp, '.');
    if (ptr == NULL)
    {
        ptr = tmp;
    }
    else
    {
        ptr = ptr + 1;
    }
    return dlsym(comp->_dll, ptr);
}

} // namespace module
} // namespace wolf