#include "coreDump.h"

#include "util/stringUtil.h"
#include "util/timestamp.h"
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#define MAX_NAME_LEN 256
#define MAX_CORE_SIZE 512
#define MAX_CORE_BUFFER_SIZE 4096
#define DUMP_DEFAULT_DIR "./dump"

/*-g -rdynamic
linux 版本
window dbughelp
*/

NS_CC_BEGIN

void coreDump::coreSignal(int dunno) {
  INST(coreDump, outStack, dunno);
  signal(dunno, SIG_DFL);
  raise(dunno);
}

void coreDump::doListen() {
  signal(SIGABRT, coreSignal);
  signal(SIGSEGV, coreSignal);
}

void coreDump::outStack(int sig) {

  std::string strTime = util::timestamp::getTimeLocal();
  std::string strFName = DUMP_DEFAULT_DIR;
  strFName += "/core." + strTime;
  strFName = util::stringUtil::replace(
      util::stringUtil::replace(strFName, " ", "_"), ":", "_");

  FILE *f = fopen(strFName.c_str(), "a");
  if (f == NULL) {
    SYSLOG_ERROR(0, "make core file fail[{}].", strFName.c_str());
    return;
  }

  int fd = fileno(f);

  struct flock fl;
  fl.l_type = F_WRLCK;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  fl.l_pid = getpid();
  fcntl(fd, F_SETLKW, &fl);

  char buffer[MAX_CORE_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));
  int count = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (count > 0) {
    buffer[count] = '\n';
    buffer[count + 1] = 0;
    fwrite(buffer, 1, count + 1, f);
  }

  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "Dump Time: %s\n", strTime.c_str());
  fwrite(buffer, 1, strlen(buffer), f);

  sprintf(buffer, "Curr thread: %u, Catch signal:%d\n", (int)pthread_self(),
          sig);
  fwrite(buffer, 1, strlen(buffer), f);

  void *DumpArray[MAX_CORE_SIZE];
  int nSize = backtrace(DumpArray, MAX_CORE_SIZE);
  sprintf(buffer, "backtrace rank = %d\n", nSize);
  fwrite(buffer, 1, strlen(buffer), f);
  if (nSize > 0) {
    if (nSize > MAX_CORE_SIZE) {
      nSize = MAX_CORE_SIZE;
    }

    char **symbols = backtrace_symbols(DumpArray, nSize);
    if (symbols != NULL) {
      for (int i = 0; i < nSize; i++) {
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

NS_CC_END
