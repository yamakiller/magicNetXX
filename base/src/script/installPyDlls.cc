#include "installPyDlls.h"
#include "base.h"

namespace wolf
{
namespace script
{
#if defined UT_PLATFORM_WINDOWS
bool installPyDlls(void)
{
    return true;
}

bool uninstallPyDlls(void)
{
    return true;
}
#else
typedef PyObject *(*pyfunc)(void);
#endif
} // namespace script
} // namespace wolf