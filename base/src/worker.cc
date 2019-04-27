#include "worker.h"
#include "scheduler.h"

namespace engine
{

worker::worker(scheduler *sch, int id) : m_id(id),
                                         m_lpsch(sch),
                                         m_waiting({false}),
                                         m_notified(false),
                                         m_runnable(nullptr)
{
    std::thread t([this] {
        worker *p = static_cast<worker *>(this);
        p->process();
    });

    m_pid.swap(t);
}

void worker::addTask(task *t)
{
    std::unique_lock<tkdeque::lock_t> lock(m_newQueue.lockRef());
    m_newQueue.pushUnLock(t);
    if (m_waiting)
    {
        m_cv.notify_all();
    }
    else
    {
        m_notified = true;
    }
}

void worker::waitCondition()
{
    std::unique_lock<tkdeque::lock_t> lock(m_runnableQueue.lockRef());
    if (m_notified)
    {
        m_notified = false;
        return;
    }

    m_waiting = true;
    m_cv.wait(lock);
    m_waiting = false;
}

void worker::notifyCondition()
{
    std::unique_lock<tkdeque::lock_t> lock(m_newQueue.lockRef());
    if (m_waiting)
    {
        m_cv.notify_all();
    }
    else
    {
        m_notified = true;
    }
}

void worker::joinWait()
{
    if (m_pid.joinable())
        m_pid.join();
}

worker *&worker::getCurrentWorker()
{
    static thread_local worker *lpworker = nullptr;
    return lpworker;
}

void worker::moveRunnable()
{
    m_runnableQueue.push(m_newQueue.popAll());
}

void worker::process()
{
    fprintf(stderr, "process run\n");
    worker::getCurrentWorker() = this;
    while (!m_lpsch->isShutdown())
    {
        m_runnable = m_runnableQueue.popFront();
        if (m_runnable != nullptr)
        {
            m_runnable->Run();
            m_waitQueue.push(m_runnable);
            m_runnable = NULL;
            continue;
        }

        moveRunnable();
        if (!m_runnableQueue.empty())
        {
            continue;
        }
        waitCondition();
    }
}

} // namespace engine