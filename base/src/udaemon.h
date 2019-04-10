#ifndef CIS_ENGINE_UDAEMON_H
#define CIS_ENGINE_UDAEMON_H

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

namespace cis
{
class udaemon
{
public:
  udaemon(const char *pidfile);
  ~udaemon();

  int init();
  int exit();

private:
  int local_check_pid();
  int local_write_pid();
  int local_redirect_fds();

private:
  char *m_cPidFile;
};

} // namespace cis

#endif
