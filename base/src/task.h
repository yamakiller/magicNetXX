#ifndef TASK_CCCCCC_H
#define TASK_CCCCCC_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace engine
{
struct task
{
    uint64_t m_id;

    task() {}
    ~task() {}

    void Run()
    {
        fprintf(stderr, "Task Run:%ld\n", pthread_self());
    }

private:
    task(task const &) = delete;
    task(task &&) = delete;
    task &operator=(task const &) = delete;
    task &operator=(task &&) = delete;
};
} // namespace engine

#endif