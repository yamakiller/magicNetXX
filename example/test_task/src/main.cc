#include <api.h>
#include <chrono>
#include <stdlib.h>
#include <string>
#include <thread>

//测试数据结构
#include <deque.h>

struct atask : public engine::util::node, public engine::shared_ref
{
    int32_t id;
    atask()
    {
    }

    ~atask()
    {
    }

private:
    atask(atask const &) = delete;
    atask(atask &&) = delete;
    atask &operator=(atask const &) = delete;
    atask &operator=(atask &&) = delete;
};

int main(int argc, char *argv[])
{

    engine::util::deque<atask, false> q;

    for (int i = 0; i < 1000; i++)
    {
        atask *p = new atask();
        p->id = 1 + i;
        q.push(p);
        engine::decrementRef<atask>(p);
    }

    fprintf(stderr, "push end\n");
    q.assertLink();

    auto l = q.popBackAll();
    auto o = l.cut(10);
    l.append(std::move(o));

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