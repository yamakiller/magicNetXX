
#include <uframework.h>
#include <uexception.h>

namespace cis
{
namespace example
{
class LoggerFramework : public uframework
{
public:
    int initialize() override
    {
        uframework::initialize();
        try
        {
            umodule *lplogger = new umodule("logger", NULL);
        }
        catch (const uexception &e)
        {
            return 1;
        }

        return 0;
    }
};

static LoggerFramework app;
} // namespace example
} // namespace cis