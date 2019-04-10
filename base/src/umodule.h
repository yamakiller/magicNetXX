#ifndef CIS_ENGINE_UMODULE_H
#define CIS_ENGINE_UMODULE_H

#include "icomponent.h"
#include "umessage.h"
#include <string>

using namespace std;

namespace cis
{
class umodule : public uobject
{
    typedef icomponent *(*component_dl_create)(void);
    typedef void (*component_dl_release)(icomponent *cmpt);

public:
    umodule(const char *strName, const char *strParam);
    ~umodule();

    inline const char *getName() { return m_strName.c_str(); }
    inline uint32_t getId() { return m_uId; }
    inline void setId(uint32_t id) { m_uId = id; }

private:
    void *local_getApi(void *lpDll, const char *strName);

private:
    uint32_t m_uId;
    string m_strName;

    icomponent *m_lpCmpt;
    void *m_lpDll;

    umq *m_lpMq;
};
} // namespace cis

#endif
