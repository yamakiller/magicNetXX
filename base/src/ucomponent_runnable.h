#ifndef CIS_ENGINE_UCOMPONENT_RUNNABLE_H
#define CIS_ENGINE_UCOMPONENT_RUNNABLE_H

#include "icomponent.h"

namespace cis
{
class ucomponent_runnable : icomponent
{
public:
    virtual int initialize(void *, const char *strParam);

    virtual void finalize();

    virtual int runOnce() = 0;

private:
    void *run(void *parm);
};

} // namespace cis

#endif
