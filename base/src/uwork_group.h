#ifndef CIS_ENGINE_UWORK_GROUP_H
#define CIS_ENGINE_UWORK_GROUP_H

#include "usingleton.h"
#include "unoncopyable.h"
#include <stdint.h>
#include <thread>
#include <vector>
#include <functional>

using namespace std;
namespace cis
{
class uwork_group : public usingleton<uwork_group> /*, unoncopyable*/
{
public:
    void initialize();

    void append(function<void *(void *)>, void *param);

    void wait();

public:
    volatile int32_t m_iShutdown;

private:
    vector<thread> m_tGroups;
};
} // namespace cis

#endif