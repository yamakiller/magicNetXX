#include <chrono>
#include <scheduler.h>
#include <stdlib.h>
#include <string>
#include <thread>

#include <deque.h>
#include <task.h>

int main(int argc, char *argv[])
{
    /*1.engine::scheduler::instance()->doStart(6);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1000000));
        for (int i = 0; i < 20; i++)
        {
            engine::scheduler::instance()->createTask();
        }

        std::string debug_info = engine::scheduler::instance()->debug();
        fprintf(stderr, "\033[2J");
        fprintf(stderr, "%s", debug_info.c_str());
    }*/

    //2.测试数据结构
    return 0;
}