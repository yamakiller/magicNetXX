#ifndef CIS_ENGINE_TEST_ACTOR_H
#define CIS_ENGINE_TEST_ACTOR_H

#include "module/actor.h"

namespace engine
{
namespace test
{
class testActor : public module::actor
{
protected:
    int32_t initialize();
    void finalize();
};
} // namespace test
} // namespace engine

#endif