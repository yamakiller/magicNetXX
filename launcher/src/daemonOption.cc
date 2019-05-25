#include "daemonOption.h"
#include <api.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

namespace engine {
daemonOption::daemonOption() {

  time_t t = time(0);
  char szDmupName[260];
  tm *ptm = localtime(&t);

  sprintf(szDmupName, "%d_%d_%d_%d_%d_%d.dmp", ptm->tm_year + 1900, ptm->tm_mon,
          ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  m_pidFile = util::stringUtil::strdup(szDmupName);

  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
}

daemonOption::~daemonOption() { util::memory::free(m_pidFile); }

int daemonOption::init() {
  int pid = local_check_pid();
  if (pid) {
    fprintf(stderr, "System is already running, pid ----->>>>> %d.\n", pid);
    return -1;
  }

#ifdef __APPLE__
  fprintf(stderr, "'daemon' is deprecated: first deprecated in OS X 10.5 "
                  "----->>>>> use launchd instead.\n");
#else
  if (daemon(1, 1)) {
    fprintf(stderr, "Can`t daemonize.\n");
    return -1;
  }
#endif

  pid = local_write_pid();
  if (pid == 0)
    return -1;

  if (local_redirect_fds())
    return -1;

  return 0;
}

int daemonOption::exit() { return unlink(m_pidFile); }

int daemonOption::local_check_pid() {
  int pid = 0;
  FILE *f = fopen(m_pidFile, "r");
  if (f == NULL)
    return 0;
  int n = fscanf(f, "%d", &pid);
  fclose(f);

  if (n != 1 || pid == 0 || pid == getpid())
    return 0;
  if (kill(pid, 0) && errno == ESRCH)
    return 0;
  return pid;
}

int daemonOption::local_write_pid() {
  FILE *f;
  int pid = 0;
  int fd = open(m_pidFile, O_RDWR | O_CREAT, 0644);
  if (fd == -1) {
    fprintf(stderr, "Can't create pidfile ----->>>>> [%s].\n", m_pidFile);
    return 0;
  }
  f = fdopen(fd, "r+");
  if (f == NULL) {
    fprintf(stderr, "Can't open pidfile ----->>>>> [%s].\n", m_pidFile);
    return 0;
  }

  if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
    int n = fscanf(f, "%d", &pid);
    fclose(f);
    if (n != 1) {
      fprintf(stderr, "Can't lock and read pidfile.\n");
    } else {
      fprintf(stderr,
              "Can't lock pidfile, lock is held by pid ----->>>>> %d.\n", pid);
    }
    return 0;
  }

  pid = getpid();
  if (!fprintf(f, "%d\n", pid)) {
    fprintf(stderr, "Can't write pid.\n");
    close(fd);
    return 0;
  }
  fflush(f);

  return pid;
}

int daemonOption::local_redirect_fds() {
  int nfd = open("/dev/null", O_RDWR);
  if (nfd == -1) {
    perror("Unable to open /dev/null: ");
    return -1;
  }
  if (dup2(nfd, 0) < 0) {
    perror("Unable to dup2 stdin(0): ");
    return -1;
  }
  if (dup2(nfd, 1) < 0) {
    perror("Unable to dup2 stdout(1): ");
    return -1;
  }
  if (dup2(nfd, 2) < 0) {
    perror("Unable to dup2 stderr(2): ");
    return -1;
  }

  close(nfd);

  return 0;
}

} // namespace engine