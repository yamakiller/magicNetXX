#include "coreDump.h"

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_NAME_LEN 64
#define MAX_CORE_SIZE 512
#define MAX_CORE_BUFFER_SIZE 4096

/*-g -rdynamic*/

namespace wolf
{

void coreDump::coreSignal(int dunno)
{
    const char *signal_str = "";
    char dunno_str[MAX_NAME_LEN] = {0};
    sprintf(dunno_str, "%d", dunno);
    switch (dunno)
    {
    case 1:
        signal_str = "SIGHUP(1)";
        break;
    case 2:
        signal_str = "SIGINT(2:CTRL_C)"; //CTRL_C
        break;
    case 3:
        signal_str = "SIGQUIT(3)";
        break;
    case 6:
    {
        signal_str = "SIGABRT(6)";
        INST(coreDump, outStack, signal_str);
    }
    break;
    case 9:
        signal_str = "SIGKILL(9)";
        break;
    case 15:
        signal_str = "SIGTERM(15 KILL)"; //kill
        break;
    case 11:
    {
        signal_str = "SIGSEGV(11)"; //SIGSEGV
        INST(coreDump, outStack, signal_str);
    }
    break;
    default:
        signal_str = "OTHER";
        break;
    }
    exit(0);
}

void coreDump::doListen()
{
    signal(SIGHUP, coreSignal);
    signal(SIGINT, coreSignal);
    signal(SIGQUIT, coreSignal);
    signal(SIGABRT, coreSignal);
    signal(SIGKILL, coreSignal);
    signal(SIGTERM, coreSignal);
    signal(SIGSEGV, coreSignal);
}

void coreDump::outStack(const char *sig)
{
    time_t tSetTime;
    time(&tSetTime);
    struct tm *ptm = localtime(&tSetTime);
    char fname[256] = {0};
    sprintf(fname, "core.%d-%d-%d_%d_%d_%d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    FILE *f = fopen(fname, "a");
    if (f == NULL)
    {
        return;
    }
    int fd = fileno(f);

    //锁定文件
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);

    //输出程序的绝对路径
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int count = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (count > 0)
    {
        buffer[count] = '\n';
        buffer[count + 1] = 0;
        fwrite(buffer, 1, count + 1, f);
    }

    //输出信息的时间
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Dump Time: %d-%d-%d %d:%d:%d\n",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    fwrite(buffer, 1, strlen(buffer), f);

    //线程和信号
    sprintf(buffer, "Curr thread: %u, Catch signal:%s\n",
            (int)pthread_self(), sig);
    fwrite(buffer, 1, strlen(buffer), f);

    //堆栈
    void *DumpArray[256];
    int nSize = backtrace(DumpArray, 256);
    sprintf(buffer, "backtrace rank = %d\n", nSize);
    fwrite(buffer, 1, strlen(buffer), f);
    if (nSize > 0)
    {
        char **symbols = backtrace_symbols(DumpArray, nSize);
        if (symbols != NULL)
        {
            for (int i = 0; i < nSize; i++)
            {
                fwrite(symbols[i], 1, strlen(symbols[i]), f);
                fwrite("\n", 1, 1, f);
            }
            free(symbols);
        }
    }

    //文件解锁后关闭
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);
    fclose(f);
}

} // namespace wolf
