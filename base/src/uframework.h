#ifndef CIS_ENGINE_UFRAMEWORK_H
#define CIS_ENGINE_UFRAMEWORK_H

#include "usingleton.h"
#include "urwlock.h"
#include <string>
#include <memory>
#include <unordered_map>

#define UMODULE_MAX_ID 65535
#define UFRAMEWORK_ID_SHIFT 24

using namespace std;

namespace cis
{
class umodule;
class uframework : public usingleton<uframework>
{
    typedef shared_ptr<umodule> ModulePtr;
    typedef unordered_map<uint32_t, ModulePtr> MapModule;
    typedef unordered_map<string, uint32_t> MapModuleId;

public:
    uframework();
    ~uframework();

    inline void setFrameworkId(uint32_t id) { m_uFrameworkId = (id << UFRAMEWORK_ID_SHIFT); };
    inline uint32_t getFrameworkId() { return (m_uFrameworkId >> UFRAMEWORK_ID_SHIFT); }

    inline void setName(const char *strName) { m_strFrameName = strName; }
    inline const char *getName() { return m_strFrameName.c_str(); }

    uint32_t registerModule(umodule *lpModule);
    void unregisterModule(uint32_t id);

    void *openDynamicL(const char *strName);

private:
    uint32_t local_mallocId();
    void local_eraseName(const char *strName);

private:
    volatile uint32_t m_uModuleMallocID;
    uint32_t m_uFrameworkId;

    MapModule m_uMapModule;
    MapModuleId m_uMapIdName;
    urwlock m_rwMapLock;

    string m_strFrameName;
    string m_strCPath;
};
} // namespace cis

#endif
