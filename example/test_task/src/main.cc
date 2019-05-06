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

struct A1
{
    int32_t c;
    int32_t d;

    A1()
    {
    }

    ~A1()
    {
    }
};

struct A2
{
    int32_t e;
    int32_t f;

    A2()
    {
    }

    ~A2()
    {
    }
};

struct CATest : public A1, public A2
{
    int32_t a;
    int32_t b;

    CATest()
    {
    }

    ~CATest()
    {
    }
};

int main(int argc, char *argv[])
{
    CATest *test = new CATest();
    A1 *pa1 = static_cast<A1 *>(test);
    A2 *pa2 = static_cast<A2 *>(test);

    fprintf(stderr, "CA:%p, A1:%p, A2:%p\n", test, pa1, pa2);

    /*engine::invDeque<atask, false> q;

    for (int i = 0; i < 1; i++)
    {
        atask *p = new atask();
        p->id = 1 + i;
        engine::invNode *test = static_cast<engine::invNode *>(p);
        fprintf(stderr, "base struct:%p, first struct:%p\n", p, test);
        //q.push(p);
    }

    fprintf(stderr, "push end\n");
    q.assertLink();*/
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