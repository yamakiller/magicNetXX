#ifndef CIS_ENGINE_UCOMPONENT_MSG_H
#define CIS_ENGINE_UCOMPONENT_MSG_H

#include "icomponent.h"
#include "umsg.h"

#include <assert.h>
#include <mutex>
#include <condition_variable>

using namespace std;
namespace cis
{
class ucomponent_msg : icomponent
{
public:
    ucomponent_msg();
    virtual ~ucomponent_msg();

    virtual int initialize(void *, const char *strParam);

    virtual void finalize();

    void wakeup();

    inline void push(umsg *lpmsg)
    {
        assert(m_lpMqs);
        m_lpMqs->push(lpmsg);
        wakeup();
    }

protected:
    virtual int runOnce(uint32_t source, int session, int type, void *data, size_t sz) = 0;

private:
    void *run(void *parm);

protected:
    umq *m_lpMqs;

    int m_sleep;
    condition_variable m_cv;
    mutex m_mutex;
};
} // namespace cis

#endif
