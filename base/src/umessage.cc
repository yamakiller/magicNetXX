#include "umessage.h"

namespace cis
{
void umq::local_drop(umsg *e)
{
    umemory::free(e->data);
    //forward error
}
} // namespace cis