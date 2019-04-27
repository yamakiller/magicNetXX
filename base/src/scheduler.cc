#include "scheduler.h"
#include <assert.h>

namespace engine
{
scheduler::scheduler() : m_taskCount({0}),
                         m_maxThreadNumber(1),
                         m_shutdown(0)
{
}

scheduler::~scheduler()
{
}

int32_t scheduler::doStart(int32_t threadNumber)
{
    if (!m_started.try_lock())
    {
        throw std::logic_error("cis engine repeated call scheduler::doStart");
    }

    if (threadNumber < 1)
        threadNumber = std::thread::hardware_concurrency();

    m_maxThreadNumber = threadNumber;

    for (int i = 0; i < m_maxThreadNumber; i++)
    {
        newWorker();
    }

    if (m_maxThreadNumber > 1)
    {
        fprintf(stderr, "---> Create Dispatcher Thread\n");
        std::thread t([this] {
            this->dispatcherWork();
        });
        m_dispatchThread.swap(t);
    }
    else
    {
        fprintf(stderr, "---> No Dispatcher Thread\n");
    }

    fprintf(stderr, "---> cis engine start mThreadNumber:%d\n", m_maxThreadNumber);
    m_started.unlock();

    return 0;
}

void scheduler::doShutdown()
{
    std::unique_lock<std::mutex> lock(m_shutdownMtx);
    m_shutdown = 1;
    size_t n = m_works.size();
    for (size_t i = 0; i < n; ++i)
    {
        auto p = m_works[i];
        if (p)
            p->notifyCondition();
    }

    if (m_dispatchThread.joinable())
        m_dispatchThread.join();
}

void scheduler::newWorker()
{
    worker *pwk = new worker(this, m_works.size());
    assert(pwk);
    m_works.push_back(pwk);
}

void scheduler::dispatcherWork()
{
    fprintf(stderr, "Start dispatcher work\n");
    while (!m_shutdown)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }

    size_t n = m_works.size();
    for (size_t i = 0; i < n; ++i)
    {
        auto p = m_works[i];
        if (p)
            p->joinWait();
    }
}

} // namespace engine