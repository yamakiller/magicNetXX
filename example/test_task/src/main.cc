#include <chrono>
#include <scheduler.h>
#include <stdlib.h>
#include <string>
#include <thread>

//测试数据结构
#include <deque2.h>

struct atask : public engine::invNode, public engine::SharedRefObject
{
    int32_t id;
    atask()
    {
    }

    ~atask()
    {
        fprintf(stderr, "~atask\n");
    }

private:
    atask(atask const &) = delete;
    atask(atask &&) = delete;
    atask &operator=(atask const &) = delete;
    atask &operator=(atask &&) = delete;
};

int main(int argc, char *argv[])
{
    engine::invDeque<atask, false> q;

    for (int i = 0; i < 1; i++)
    {
        atask *p = new atask();
        p->id = 1 + i;
        engine::invNode *test = static_cast<engine::invNode *>(p);
        fprintf(stderr, "base struct:%p, first struct:%p\n", p, test);
        //q.push(p);
    }

    fprintf(stderr, "push end\n");
    q.assertLink();
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