#ifndef WOLF_MOBJECT_H
#define WOLF_MOBJECT_H

#include "memory.h"

namespace wolf
{
namespace util
{
class mobject
{
public:
    mobject() = default;
    virtual ~mobject() = default;

public:
    void *operator new(size_t size)
    {
        return util::memory::malloc(size);
    }

    void *operator new[](size_t size)
    {
        return util::memory::malloc(size);
    }

    void operator delete(void *p)
    {
        util::memory::free(p);
    }

    void operator delete[](void *p)
    {
        util::memory::free(p);
    }
};
} // namespace util
} // namespace wolf

#endif