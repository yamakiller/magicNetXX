
#include <base.h>

namespace cis
{
namespace example
{
class LoggerFramework : public uframework
{
public:
    int initialize(ucommand_optline &opt) override
    {
        FRAMEWORK_INITIALIZE_ASSERT(opt);
        return 0;
    }
};

static LoggerFramework app;
} // namespace example
} // namespace cis