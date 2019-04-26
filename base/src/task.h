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
};
} // namespace engine

#endif