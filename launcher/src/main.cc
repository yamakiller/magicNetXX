#include "commandLineOption.h"
#include "daemonOption.h"
#include <framework.h>
#include <stdlib.h>
#include <string>

void doPrintVersion()
{
  fprintf(stderr, "cis engine %d.%d.%d\n", ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);
  fprintf(stderr, "Copyright (C) 2019 Free Software Foundation, Inc.\n");
  fprintf(stderr, "This is free software; see the source for copying conditions. There is NO\n");
  fprintf(stderr, "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
}

void doPrintHelp()
{
  fprintf(stderr, "Usage: wolf [options] file...\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "   -help                Display this information.\n");
  fprintf(stderr, "   -version             Display version information.\n");
  fprintf(stderr, "   -d                   The program will run in the background.\n");
  fprintf(stderr, "   -p <filepath>        Set the configuration file path.\n");
  fprintf(stderr, "other, please see:\n");
  fprintf(stderr, "<file:README.md>.\n");
}

int main(int argc, char *argv[])
{
  wolf::commandLineOption option;
  wolf::daemonOption *daemon_ptr = nullptr;
  option.setOption("d", false);
  option.setOption("help", false);
  option.setOption("version", false);

  if (!option.parse(argc, argv))
  {
    fprintf(stderr, "command line parse failed.\n");
    return -1;
  }

  if (option.isSet("d"))
  {
    daemon_ptr = new wolf::daemonOption();
    if (daemon_ptr->init() != 0)
    {
      return -1;
    }
  }
  else if (option.isSet("help"))
  {
    doPrintHelp();
    return -1;
  }
  else if (option.isSet("version"))
  {
    doPrintVersion();
    return -1;
  }

  wolf::framework *framework = wolf::framework::instance();
  if (framework == 0)
  {
    return 0;
  }

  if (!framework->doInit(&option))
  {
    return 0;
  }

  framework->startLoop();

  framework->doUnInit();

  if (daemon_ptr)
  {
    daemon_ptr->exit();
    delete daemon_ptr;
  }
}