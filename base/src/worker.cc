#include "worker.h"

namespace engine
{

worker::worker(scheduler *sch, int id) : m_id(id),
                                         m_lpsch(sch),
                                         m_waiting({false}),
                                         m_notified(false),
                                         m_runnable(nullptr)
{
    m_pid = std::thread([this] {
        worker *p = static_cast<worker *>(this);
        p->process();
    });
}

void worker::addTask(task *t)
{
    std::unique_lock<deque::lock_t> lock(m_newQueue.lockRef());
    m_newQueue.pushWithoutLock(t);
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
    std::unique_lock<deque::lock_t> lock(m_runnableQueue.lockRef());
    if (m_notified)
    {
        m_notified = false;
        return;
    }

    m_waiting = true;
    m_cv.wait();
    m_waiting = false;
}

void worker::notifyCondition()
{
    std::unique_lock<deque::lock_t> lock(m_newQueue.lockRef());
    if (m_waiting)
    {
        m_cv.notify_all();
    }
    else
    {
        m_notified = true;
    }
}

worker *worker::getCurrentWorker()
{
    static thread_local worker *lpworker = nullptr;
    return lpworker;
}

void worker::moveRunnable()
{
    m_runnableQueue.push
}

void worker::process()
{
    worker::getCurrentWorker() = this;
    while (!m_lpsch->isShutdown())
    {
        m_runnable = m_runnableQueue.popFront();
        if (m_runnable != nullptr)
        {
            m_runnable->Run();
            //--结束处理,放回到等待池里面
            continue;
        }

        //把新任务搬过来
        waitCondition();
        //
    }
}

} // namespace engine