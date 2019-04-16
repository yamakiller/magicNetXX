#include <unistd.h>
#include <udaemon.h>
#include <ilog.h>
#include <uframework.h>
#include "ucommand_optline.h"

using namespace cis;

int main(int argc, char *argv[])
{
    ucommand_optline option;
    udaemon *lpDaemon = NULL;
    if (!option.parse(argc, argv))
    {
        LOG_ERROR(0, "command line parse failed.");
        return 0;
    }

    if (option.isSet("d"))
    {
        lpDaemon = new udaemon(option.getOption("d").c_str());
        lpDaemon->init();
    }

    if (INSTGET(uframework) == 0)
    {
        LOG_ERROR(0, "framework instance is nill.");
        return 0;
    }

    if (INST(uframework, initialize, option) != 0)
    {
        LOG_ERROR(0, "framework initialize fail.");
        return 0;
    }

    INST(uframework, startLoop);

    INST(uframework, finalize);

    if (lpDaemon != NULL)
    {
        lpDaemon->exit();
        delete lpDaemon;
    }

    return 0;
}