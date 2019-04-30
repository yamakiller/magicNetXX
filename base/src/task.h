#ifndef TASK_CCCCCC_H
#define TASK_CCCCCC_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace engine
{
struct Releaser
{
    typedef void (*func_t)(struct task *ptr, void *arg);
    func_t _func;
    void *_arg;

    Releaser() : _func(nullptr), _arg(nullptr) {}
    Releaser(func_t func, void *arg) : _func(func), _arg(arg) {}

    inline void operator()(struct task *ptr)
    {
        if (_func)
            _func(ptr, _arg);
    }
};

struct task
{
    uint64_t m_id;
    Releaser m_releaser;

    task() {}
    ~task()
    {
        m_releaser(this);
    }

    void Run()
    {
        for (int i = 0; i < 100; i++)
        {
        }
    }

    void setReleaser(struct Releaser r)
    {
        m_releaser = r;
    }

private:
    task(task const &) = delete;
    task(task &&) = delete;
    task &operator=(task const &) = delete;
    task &operator=(task &&) = delete;
};
} // namespace engine

#endif