#include <chrono>
#include <scheduler.h>
#include <stdlib.h>
#include <thread>

int main(int argc, char *argv[])
{
    engine::scheduler::instance()->doStart(6);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1000000));
        for (int i = 0; i < 20; i++)
        {
            engine::scheduler::instance()->createTask();
        }
        fprintf(stderr, "end-------------------------\n");
    }
    return 0;
}