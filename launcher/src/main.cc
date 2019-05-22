#include "commandLineOption.h"
#include "daemonOption.h"
#include <framework.h>
#include <stdlib.h>
#include <string>

int main(int argc, char *argv[]) {
  engine::commandLineOption option;
  engine::daemonOption *daemon_ptr = nullptr;
  option.setOption("d", false);

  if (!option.parse(argc, argv)) {
    fprintf(stderr, "command line parse failed.\n");
    return -1;
  }

  if (option.isSet("d")) {
    daemon_ptr = new engine::daemonOption();
    if (daemon_ptr->init() != 0) {
      return -1;
    }
  }

  engine::framework *framework = engine::framework::instance();
  if (framework == 0) {
    return 0;
  }

  if (!framework->doInit()) {
    return 0;
  }

  framework->startLoop();

  framework->doUnInit();

  if (daemon_ptr) {
    daemon_ptr->exit();
    delete daemon_ptr;
  }
}