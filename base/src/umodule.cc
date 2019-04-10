#include "umodule.h"
#include "uframework.h"
#include "uexception.h"

#include "ilog.h"

namespace cis
{

umodule::umodule(const char *strName, const char *strParam) : m_strName(strName)
{
    void *dl = INST(uframework, openDynamicL, strName);
    if (dl == NULL)
    {
        throw uexception("");
        return;
    }

    component_dl_create create = local_getApi(dl, strName, "create");
    if (create == NULL)
    {
        PRINT_FATAL("{} get api {} fail", strName, "create");
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
        INST(uframework, registerModule, this);
    }
    catch (uexception &e)
    {
        PRINT_FATAL("{}", e.getMessage());
        throw uexception("");
        return;
    }

    m_lpDll = dl;
    m_lpCmpt = icmpt;

    m_lpMq = new umq(m_uId);
    assert(m_lpMq);

    int r = m_lpCmpt->initialize(this, strParam);
    if (r == 0)
    {
        //输出到日志启动成功
    }
    else
    {
        //输出到日志启动失败
        throw uexception("");
    }
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