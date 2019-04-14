#include "umodule.h"
#include "uframework.h"
#include "uexception.h"
#include "ilog.h"

#include "umsg.h"
#include "ucomponent_msg.h"
#include "umodule_mgr.h"

#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

namespace cis
{

umodule::umodule(const char *strName, const char *strParam) : m_strName(strName)
{
    void *dl = INST(umodule_mgr, openDynamicL, strName);
    if (dl == NULL)
    {
        throw uexception("");
        return;
    }

    component_dl_create create = (component_dl_create)local_getApi(dl, strName, "_create");
    if (create == NULL)
    {
        LOG_CRITICAL(0, "{} get api {} fail", strName, "create");
        throw uexception("");
        return;
    }

    icomponent *icmpt = create();
    if (icmpt == NULL)
    {
        throw uexception("");
        return;
    }

    try
    {
        INST(umodule_mgr, registerModule, this);
    }
    catch (uexception &e)
    {
        LOG_CRITICAL(0, "{}", e.getMessage());
        throw uexception("");
        return;
    }

    m_lpDll = dl;
    m_lpCmpt = icmpt;

    int r = m_lpCmpt->initialize(this, strParam);
    if (r == 0)
    {
        LOG_INFO(m_uId, "LAUNCH {} {}", strName, strParam ? strParam : "");
    }
    else
    {
        LOG_INFO(m_uId, "FAILED Launch {}", strName);
        throw uexception("");
    }
}

umodule::~umodule()
{
    RELEASE(m_lpCmpt);
    m_lpDll = NULL;
}

int umodule::push(struct umsg *msg)
{
    ucomponent_msg *lpcmpt = dynamic_cast<ucomponent_msg *>(m_lpCmpt);
    if (lpcmpt == NULL)
    {
        return -1;
    }

    lpcmpt->push(msg);
    return 0;
}

void umodule::wakeup()
{
    ucomponent_msg *lpcmpt = dynamic_cast<ucomponent_msg *>(m_lpCmpt);
    if (lpcmpt == NULL)
    {
        return;
    }
    lpcmpt->wakeup();
}

void *umodule::local_getApi(void *lpDll, const char *strName, const char *strNameApi)
{
    size_t name_size = strlen(strName);
    size_t api_size = strlen(strNameApi);
    char tmp[name_size + api_size + 1];
    memcpy(tmp, strName, name_size);
    memcpy(tmp + name_size, strNameApi, api_size + 1);

    char *ptr = strrchr(tmp, '.');
    if (ptr == NULL)
    {
        ptr = tmp;
    }
    else
    {
        ptr = ptr + 1;
    }
    return dlsym(lpDll, ptr);
}

} // namespace cis