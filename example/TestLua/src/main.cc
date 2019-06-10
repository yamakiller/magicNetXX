
#include <string>
#include <api.h>

namespace wolf
{

namespace example
{

class testLuaFramework : public framework
{
private:
protected:
    virtual bool initialize(const commandLineOption *opt)
    {
        module::actor *pt = new module::actor();
        if (pt->doInit("lnlua", (void *)"123456") == 0)
        {
            fprintf(stderr, "创建失败\n");
        }
        return true;
    }

    virtual bool loop() { return true; }

    virtual void finalize() {}
};

static testLuaFramework app;

} // namespace example
} // namespace wolf
