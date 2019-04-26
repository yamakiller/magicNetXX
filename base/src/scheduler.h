#ifndef CIS_ENGINE_SCHEDULER_H
#define CIS_ENGINE_SCHEDULER_H

#include "singleton.h"
#include <mutex>
#include <stdint.h>
#include <thread>

namespace engine
{
class scheduler : public singleton<scheduler>
{
public:
    scheduler();
    ~scheduler();

    inline int32_t isShutdown() { return m_shutdown; }

private:
    int32_t m_minThread;
    int32_t m_maxThread;

    std::thread m_dispatchThread; //调度线程

    std::mutex m_shutdownMtx;
    int32_t m_shutdown;
};
} // namespace engine

#endif