#ifndef CIS_ENGINE_OBJECT_H
#define CIS_ENGINE_OBJECT_H

#include "macro.h"
#include "umemory.h"
#include <stddef.h>
#include <stdint.h>

namespace cis
{
class uobject
{
public:
    uobject() {}
    ~uobject() {}

    void *operator new(size_t size)
    {
        return umemory::malloc(size);
    }

    void *operator new[](size_t size)
    {
        return umemory::malloc(size);
    }

    void operator delete(void *p)
    {
        umemory::free(p);
    }

    void operator delete[](void *p)
    {
        umemory::free(p);
    }
};

} // namespace cis

#endif