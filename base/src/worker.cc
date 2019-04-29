#include "worker.h"
#include "clock.h"
#include "scheduler.h"

namespace engine
{

worker::worker(scheduler *sch, int id) : m_id(id),
                                         m_lpsch(sch),
                                         m_waiting({false}),
                                         m_waittick(0),
                                         m_ntsTick(0),
                                         m_ntsMark(0),
                                         m_nts(0),
                                         m_notified(false),
                                         m_runnable(nullptr)
{
    std::thread t([this] {
        worker *p = static_cast<worker *>(this);
        p->process();
    });

    m_pid.swap(t);
}

worker::~worker()
{
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

int32_t worker::isBusy()
{
    if (!m_ntsMark || m_ntsMark != m_nts)
        return 0;
    return clock::instance()->now() > m_ntsTick + 2000 ? true : false;
}

void worker::busyMark()
{
    if (m_runnable && m_ntsMark != m_nts)
    {
        m_ntsMark = m_nts;
        m_ntsTick = clock::instance()->now();
    }
}

int32_t worker::isWaiting()
{
    if (!m_waiting)
        return 0;
    return (clock::instance()->now() - m_waittick) > 10 ? true : false;
}

size_t worker::getRunnableNum()
{
    return m_runnableQueue.size() + m_newQueue.size();
}

void worker::waitCondition()
{
    std::unique_lock<tkdeque::lock_t> lock(m_runnableQueue.lockRef());
    if (m_notified)
    {
        m_notified = false;
        return;
    }

    m_waittick = clock::instance()->now();
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

void worker::gc() //需要修改
{
    std::queue<task *> *list = m_gccQueue.popAll();
    while (list->empty())
    {
        task *p = list->front();
        list->pop();
        delete p;
    }
    delete list;
}

void worker::process()
{
    fprintf(stderr, "process run :%ld\n", pthread_self());
    worker::getCurrentWorker() = this;
    while (!m_lpsch->isShutdown())
    {
        m_runnable = m_runnableQueue.popFront();
        if (m_runnable != nullptr)
        {
            ++m_nts;
            m_runnable->Run();
            if (m_gccQueue.size() > 16)
                gc();
            m_gccQueue.push(m_runnable);
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

std::queue<task *> *worker::steal(std::size_t n)
{
    int other = 0;
    std::queue<task *> *steal_q = m_newQueue.popBack(n);
    if (n > 0)
    {
        if (steal_q->size() >= n)
            return steal_q;
        other = n - steal_q->size();
    }

    std::queue<task *> *steal_q_1 = m_runnableQueue.popBack(other);
    while (!steal_q->empty())
    {
        steal_q_1->push(steal_q->front());
        steal_q->pop();
    }
    delete steal_q;
    return steal_q_1;
}

} // namespace engine