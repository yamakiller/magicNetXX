#ifndef CIS_ENGINE_UFRAMEWORK_H
#define CIS_ENGINE_UFRAMEWORK_H

#include "usingleton.h"
#include "unoncopyable.h"
#include "umodule_mgr.h"
#include <string>

#define UFRAMEWORK_ID_SHIFT 24

using namespace std;

namespace cis
{
class uframework
{
public:
    uframework();
    virtual ~uframework();

    static uframework *instance();

    virtual int initialize();

    virtual void finalize();

    void startLoop();

    inline void setFrameworkId(uint32_t id) { m_uFrameworkId = (id << UFRAMEWORK_ID_SHIFT); };
    inline uint32_t getFrameworkId() { return (m_uFrameworkId >> UFRAMEWORK_ID_SHIFT); }

    inline void setName(const char *strName) { m_strFrameName = strName; }
    inline const char *getName() { return m_strFrameName.c_str(); }

private:
    uint32_t m_uFrameworkId;
    string m_strFrameName;
};
} // namespace cis

#endif
