//WorkSteal

#ifndef CIS_ENGINE_WORKER_H
#define CIS_ENGINE_WORKER_H

#include "actor.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "deque.h"
#include "task.h"

namespace engine
{

class scheduler;

class worker
{
    friend class scheduler;

public:
    worker(scheduler *sch, int id);
    ~worker();

    inline int Id() { return m_id; }

    static worker *getCurrentWorker();

    void addTask(task *t);

private:
    void waitCondition();

    void notifyCondition();

    void moveRunnable();

    void process();

private:
    int m_id;
    scheduler *m_lpsch;
    std::thread m_pid;
    //条件变量
    std::condition_variable_any m_cv;
    std::atomic_bool m_waiting;
    bool m_notified;
    //--------------------
    task *m_runnable;
    deque<task *, true> m_runnableQueue;
    deque<task *, true> m_newQueue;
    deque<task *, true> m_waitQueue;
};
} // namespace engine

#endif
