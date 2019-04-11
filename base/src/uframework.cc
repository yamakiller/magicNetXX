#include "uframework.h"
#include "umodule.h"
#include "atomic.h"
#include "uexception.h"
#include "ilog.h"
#include "uwork_group.h"

#include <string.h>

#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <functional>

namespace cis
{

uframework::uframework() : m_uModuleMallocID(1)
{
}

int uframework::initialize(const char *strCmdLine)
{
    INST(uwork_group, initialize);

    return 0;
}

void uframework::startup()
{
    INST(uwork_group, append, bind(&uframework::local_run, this, std::placeholders::_1), NULL);
    INST(uwork_group, wait);
}

uint32_t uframework::registerModule(umodule *lpModule)
{
    uint32_t module_id = (local_mallocId() & m_uFrameworkId);
    m_rwMapLock.wlock();
    if (!m_uMapModule.empty())
    {
        auto it = m_uMapModule.find(module_id);
        if (it != m_uMapModule.end())
        {
            m_rwMapLock.unlock();
            throw uexception("Error Register Module ID Repeat");
            return 0;
        }
    }

    if (!m_uMapIdName.empty())
    {
        auto it = m_uMapIdName.find(lpModule->getName());
        if (it != m_uMapIdName.end())
        {
            m_rwMapLock.unlock();
            throw uexception("Error Register Module Name Already exist");
            return 0;
        }
    }

    lpModule->setId(module_id);

    m_uMapModule.insert(pair<uint32_t, ModulePtr>(module_id, ModulePtr(lpModule)));
    m_uMapIdName.insert(pair<string, uint32_t>(lpModule->getName(), module_id));
    m_rwMapLock.unlock();

    return module_id;
}

void uframework::unregisterModule(uint32_t id)
{

    m_rwMapLock.wlock();
    if (!m_uMapModule.empty())
    {
        auto it = m_uMapModule.find(id);
        if (it != m_uMapModule.end())
        {
            local_eraseName(it->second->getName());
            m_uMapModule.erase(it);
        }
    }
    m_rwMapLock.unlock();
}

uint32_t uframework::getModuleId(const char *strName)
{
    uint32_t result = 0;
    m_rwMapLock.rlock();
    if (m_uMapIdName.empty())
    {
        m_rwMapLock.unlock();
        return result;
    }

    auto it = m_uMapIdName.find(strName);
    if (it == m_uMapIdName.end())
    {
        m_rwMapLock.unlock();
        return result;
    }
    result = it->second;
    m_rwMapLock.unlock();

    return result;
}

uframework::ModulePtr uframework::getModule(const uint32_t id)
{
    ModulePtr result = nullptr;
    m_rwMapLock.rlock();
    if (m_uMapModule.empty())
    {
        m_rwMapLock.unlock();
        return result;
    }

    auto it = m_uMapModule.find(id);
    if (it == m_uMapModule.end())
    {
        m_rwMapLock.unlock();
        return result;
    }
    result = it->second;
    m_rwMapLock.unlock();
    return result;
}

void *uframework::openDynamicL(const char *strName)
{
    const char *l;
    const char *path = m_strCPath.c_str();
    const char *name = strName;

    size_t path_size = strlen(path);
    size_t name_size = strlen(name);

    int sz = path_size + name_size;

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
            PRINT_ERROR("Invalid cpath：{}", m_strCPath.c_str());
            exit(1);
        }

        dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
        path = l;
    } while (dl == NULL);

    if (dl == NULL)
    {
        PRINT_ERROR("try open {} failed : {}", name, dlerror());
    }

    return dl;
}

uint32_t uframework::local_mallocId()
{
    uint32_t current_id, new_id;
    while (true)
    {
        current_id = m_uModuleMallocID;
        new_id = current_id + 1;
        if (new_id >= UMODULE_MAX_ID)
        {
            new_id = 1;
        }

        if (ATOM_CAS(&m_uModuleMallocID, current_id, new_id))
            break;
    }

    return new_id;
}

void uframework::local_eraseName(const char *strName)
{
    if (m_uMapIdName.empty())
    {
        return;
    }

    auto it = m_uMapIdName.find(strName);
    if (it != m_uMapIdName.end())
    {
        m_uMapIdName.erase(it);
    }
}

void uframework::local_shutdown()
{
    uwork_group::instance()->m_iShutdown = 1;
    ModulePtr ptr = nullptr;
    for (auto it = m_uMapModule.begin(); it != m_uMapModule.end(); ++it)
    {
        ptr = it->second;
        ptr->wakeup();
    }
}

} // namespace cis