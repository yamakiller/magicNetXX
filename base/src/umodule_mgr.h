#ifndef CIS_ENGINE_UMODULE_MGR_H
#define CIS_ENGINE_UMODULE_MGR_H

#include "umodule.h"
#include "usingleton.h"
#include "unoncopyable.h"
#include "urwlock.h"
#include <memory>
#include <stdint.h>
#include <unordered_map>

using namespace std;

namespace cis
{
class umodule_mgr : public usingleton<umodule_mgr>, public unoncopyable
{
    typedef shared_ptr<umodule> ModulePtr;
    typedef unordered_map<uint32_t, ModulePtr> MapModule;
    typedef unordered_map<string, uint32_t> MapModuleId;

public:
    int initialize();

    void finalize();

    uint32_t registerModule(umodule *lpModule);
    void unregisterModule(uint32_t id);

    uint32_t getModuleId(const char *strName);
    ModulePtr getModule(const uint32_t id);

    void *openDynamicL(const char *strName);

    void setCPath(const char *strPath);

private:
    uint32_t local_mallocId();
    void local_eraseAll();
    void local_eraseName(const char *strName);

private:
    volatile uint32_t m_uModuleMallocID;
    MapModule m_uMapModule;
    MapModuleId m_uMapIdName;
    urwlock m_rwMapLock;

    string m_strCPath;
};
} // namespace cis

#endif
