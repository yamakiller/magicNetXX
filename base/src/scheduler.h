#ifndef CIS_ENGINE_SCHEDULER_H
#define CIS_ENGINE_SCHEDULER_H

#include <mutex>
#include <stdint.h>
#include <thread>
#include <vector>

#include <string>

#include "noncopyable.h"
#include "singleton.h"
#include "worker.h"

namespace engine
{

class scheduler : public singleton<scheduler>, public noncopyable
{
    friend class worker;

public:
    static const int g_ulimitedMaxThreadNumber = 40960;

    int32_t doStart(int32_t threadNumber);

    void doShutdown();

    inline int32_t isShutdown()
    {
        return m_shutdown;
    }

    void createTask();

    scheduler();
    ~scheduler();

    std::string debug();

protected:
    scheduler(scheduler const &) = delete;
    scheduler(scheduler &&) = delete;
    scheduler &operator=(scheduler const &) = delete;
    scheduler &operator=(scheduler &&) = delete;

    void addTask(task *t);

    static void releaseTask(shared_ref *t, void *arg);

private:
    void newWorker();

    void dispatcherWork();

    void timeTick();

private:
    std::vector<worker *> m_works;
    spinlock m_started;

    atomic_t<uint32_t> m_taskCount;

    int32_t m_maxThreadNumber;

    std::thread m_dispatchThread; //调度线程
    std::thread m_timeThread;

    std::mutex m_shutdownMtx;
    int32_t m_shutdown;
};
} // namespace engine

#endif