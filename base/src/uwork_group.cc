#include "uwork_group.h"
#include <assert.h>

namespace cis
{
void uwork_group::initialize()
{
    m_iShutdown = 0;
}

void uwork_group::append(function<void *(void *)> func, void *param)
{
    m_tGroups.push_back(thread(func, param));
}

void uwork_group::wait()
{
    for (auto it = m_tGroups.begin(); it != m_tGroups.end(); ++it)
    {
        if ((*it).joinable())
            (*it).join();
    }
    m_tGroups.clear();
}

} // namespace cis