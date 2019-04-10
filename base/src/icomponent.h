#ifndef CIS_ENGINE_ICOMPONENT_H
#define CIS_ENGINE_ICOMPONENT_H

#include "uobject.h"

namespace cis
{
class icomponent : public uobject
{
public:
    virtual ~icomponent() {}

    virtual int initialize(void *, const char *strParam) = 0;
};
} // namespace cis

#endif
