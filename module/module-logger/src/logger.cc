
#include <ucomponent_msg.h>

using namespace cis;

class logger : public ucomponent_msg
{
public:
    int initialize(void *lpParam, const char *strParam)
    {
        ucomponent_msg::initialize(lpParam, strParam);

        if (strParam && strcmp(strParam, "") != 0)
        {
            handle__ = fopen(strParam, "w");
            if (!handle__)
                return 1;
            filename__ = umemory::strdup(strParam);
            strcpy(filename__, strParam);
            close__ = 1;
        }
        else
            handle__ = stdout;

        return 0;
    }

    void finalize()
    {
    }

protected:
    int runOnce(uint32_t source, int session, int type, void *data, size_t sz)
    {
        switch (type)
        {
        case (int)MsgType::T_SIGNAL:
            if (filename__)
                handle__ = freopen(filename__, "a", handle__);
            break;
        case (int)MsgType::T_TEXT:
            //fprintf(handle__, "[:%08x]", source);
            fwrite(data, sz, 1, handle__);
            fprintf(handle__, "\n");
            fflush(handle__);
            break;
        }

        return 0;
    }

private:
    FILE *handle__;
    char *filename__;
    int close__;
};

extern "C" logger *
logger_create()
{
    logger *inst = new logger();
    return inst;
}

extern "C" void logger_release(logger *inst)
{
    delete inst;
    inst = NULL;
}