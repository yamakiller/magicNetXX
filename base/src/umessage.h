#ifndef CIS_ENGINE_UMESSAGE_H
#define CIS_ENGINE_UMESSAGE_H

#include "iq.h"
#include <stdint.h>

namespace cis
{
class imsg
{
public:
    virtual ~imsg() {}
    enum class MSG_TYPE
    {
        T_TEXT = 1,
        T_SIGNAL = 2,
        T_TIMEOUT = 3,
        T_ERROR = 4,
        T_USER,
    };
};

struct umsg
{
    uint32_t source;
    uint32_t target;
    int session;
    void *data;
    size_t sz;
};

class umq : public iq<umsg>
{
public:
    umq(uint32_t uId) : iq<umsg>(), m_uParentId(uId) {}

protected:
    void local_drop(umsg *e);

private:
    uint32_t m_uParentId;
};
} // namespace cis

#endif
