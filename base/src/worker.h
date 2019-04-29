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

    static worker *&getCurrentWorker();

    void addTask(task *t);

    std::queue<task *> *steal(std::size_t n);

    int32_t isBusy();

    int32_t isWaiting();

    size_t getRunnableNum();

private:
    void waitCondition();

    void notifyCondition();

    void joinWait();

    void moveRunnable();

    void gc();

    void process();

private:
    int m_id;
    scheduler *m_lpsch;
    std::thread m_pid;
    //条件变量
    std::condition_variable_any m_cv;
    std::atomic_bool m_waiting;
    volatile int64_t m_waittick;

    volatile int64_t m_ntsTick = 0;
    volatile uint64_t m_ntsMark = 0;
    volatile uint64_t m_nts = 0;
    bool m_notified;
    //--------------------
    task *m_runnable;
    typedef deque<task *, true> tkdeque;
    tkdeque m_runnableQueue;
    tkdeque m_newQueue;
    tkdeque m_waitQueue;

    typedef deque<task *, false> untkdeque;
    untkdeque m_gccQueue;
};
} // namespace engine

#endif
