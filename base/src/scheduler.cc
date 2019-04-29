#include "scheduler.h"
#include "clock.h"
#include <assert.h>
#include <map>

namespace engine
{

inline atomic_t<unsigned long long> &getIdFactory()
{
    static atomic_t<unsigned long long> idFactory;
    return idFactory;
}

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

    {
        std::thread t([this {
            this->timeTick();
        }]);
        m_timeThread.swap(t);
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
    if (m_timeThread.joinable())
        m_timeThread.join();
}

void scheduler::createTask()
{
    task *t = new task();
    t->m_id = ++getIdFactory();
    //设置委托释放器

    addTask(t);
}

void scheduler::addTask(task *t)
{
    std::size_t pcount = m_works.size();
    std::size_t idx = rand() % pcount;
    m_works[idx]->addTask(t);
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

        size_t workCount = m_works.size();
        size_t totalLoadaverage = 0;
        typedef std::multimap<size_t, size_t> LoadMap;
        LoadMap loadMaps;
        //1.统计繁忙中的线程
        std::map<size_t, size_t> busyings;
        for (size_t i = 0; i < workCount; i++)
        {
            auto w = m_works[i];
            if (w->isBusy())
            {
                busyings[i] = w->getRunnableNum();
            }
        }

        //2. 计算负载情况
        for (std::size_t i = 0; i < pcount; i++)
        {
            auto w = m_works[i];
            size_t loadaverage = w->getRunnableNum();
            totalLoadaverage += loadaverage;

            loadMaps.insert(LoadMap::value_type{loadaverage, i});
            p->busyMark();

            if (loadaverage > 0 && p->isWaiting())
            {
                p->notifyCondition();
            }
        }

        //3. 东西
        // 阻塞线程的任务steal出来
        {
            std::queue<task *> tasks;
            for (auto &kv : busyings)
            {
                auto p = m_works[kv.first];

                std::queue<task *> *tmp = p->steal(0);
                while (!tmp->empty())
                {
                    tasks->push(tmp->front());
                    tmp->pop();
                }
                delete tmp;
            }

            //分发任务
            if (!tasks->empty())
            {
                auto range = loadMaps.equal_range(loadMaps.begin()->first);
                std::size_t avg = tasks.size() / std::distance(range.first, range.second);
                if (avg == 0)
                    avg = 1;

                LoadMap newLoadMaps;
                for (auto it = range.second; it != loadMaps.end(); ++it)
                {
                    newLoadMaps.insert(*it);
                }

                for (auto it = range.first; it != range.second; ++it)
                {
                    //std::queue<task *> in = tasks.cut(avg);
                    //if (in.empty())
                    //    break;

                    //auto p = m_works[it->second];
                    //p->addTask(std::move(in));
                    //.insert(LoadMap::value_type{p->getRunnableNum(), it->second});
                }

                //if (!tasks.empty())
                //    m_works[range.first->second]->addTask(std::move(tasks));

                for (auto it = range.first; it != range.second; ++it)
                {
                    auto p = m_works[it->second];
                    newLoadMaps.insert(LoadMap::value_type{p->getRunnableNum(), it->second});
                }
                newLoadMaps.swap(loadMaps);
            }
        }

        // 如果还有在等待的线程, 从任务多的线程中拿一些给它
        if (loadMaps.begin()->first == 0)
        {
            auto range = loadMaps.equal_range(loadMaps.begin()->first);
            std::size_t waitN = std::distance(range.first, range.second);
            if (waitN == loadMaps.size())
            {
                // 都没任务了, 不用偷了
                continue;
            }

            auto maxP = m_works[loadMaps.rbegin()->second];
            std::size_t stealN = (std::min)(maxP->getRunnableNum() / 2, waitN * 1024);
            auto tasks = maxP->steal(stealN);
            if (tasks.empty())
                continue;

            std::size_t avg = tasks.size() / waitN;
            if (avg == 0)
                avg = 1;

            for (auto it = range.first; it != range.second; ++it)
            {
                /* SList<Task> in = tasks.cut(avg);
                if (in.empty())
                    break;

                auto p = m_works[it->second];
                p->AddTask(std::move(in));*/
            }
            //if (!tasks.empty())
            //    m_works[range.first->second]->addTask(std::move(tasks));
        }
    }

    size_t n = m_works.size();
    for (size_t i = 0; i < n; ++i)
    {
        auto p = m_works[i];
        if (p)
            p->joinWait();
    }
}

void scheduler::timeTick()
{
    fprintf(stderr, "Start time work\n");
    while (!m_shutdown)
    {
        clock::instance()->tick();
        std::this_thread::sleep_for(std::chrono::microseconds(2500));
    }
}

} // namespace engine