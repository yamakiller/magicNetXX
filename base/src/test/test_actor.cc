#include "test_actor.h"

namespace engine
{
namespace test
{
int32_t testActor::initialize()
{
    setRunFunc([](module::message *msg) -> int32_t {
        SYSLOG_INFO(1, "测试一下");
        return 0;
    });
    return 0;
}

void testActor::finalize()
{
}

} // namespace test
} // namespace engine
