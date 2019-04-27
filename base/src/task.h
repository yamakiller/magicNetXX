#ifndef TASK_CCCCCC_H
#define TASK_CCCCCC_H

#include <stdio.h>
#include <string.h>

namespace engine
{
struct task
{

    void Run()
    {
        fprintf(stderr, "Task Run\n");
    }

private:
    task(task const &) = delete;
    task(task &&) = delete;
    task &operator=(task const &) = delete;
    task &operator=(task &&) = delete;
};
} // namespace engine

#endif