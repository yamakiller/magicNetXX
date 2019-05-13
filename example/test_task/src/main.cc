#include <api.h>
#include <chrono>
#include <stdlib.h>
#include <string>
#include <thread>

int main(int argc, char *argv[])
{
    engine::scheduler::instance()->doStart(6);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1000000));
        for (int i = 0; i < 30; i++)
        {
            engine::scheduler::instance()->createTask();
        }

        std::string debug_info = engine::scheduler::instance()->debug();
        fprintf(stderr, "\033[2J");
        fprintf(stderr, "%s", debug_info.c_str());
    }

    //2.测试数据结构
    return 0;
}