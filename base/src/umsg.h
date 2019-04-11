#ifndef CIS_ENGINE_UMSG_H
#define CIS_ENGINE_UMSG_H

#include "iq.h"
#include <stdint.h>

#define MESSAGE_TYPE_MASK (SIZE_MAX >> 8)
#define MESSAGE_TYPE_SHIFT ((sizeof(size_t) - 1) * 8)

#define MSG_PACK(len, type) (len | ((size_t)type << MESSAGE_TYPE_SHIFT))
#define MSG_UNPACK_TYPE(sz) (sz >> MESSAGE_TYPE_SHIFT)
#define MSG_UNPACK_SIZE(sz) (sz & MESSAGE_TYPE_MASK)

namespace cis
{

enum class MsgType
{
    T_TEXT = 1,
    T_SIGNAL = 2,
    T_TIMEOUT = 3,
    T_ERROR = 4,
    T_USER,
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
