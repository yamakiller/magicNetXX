#ifndef CIS_ENGINE_USIGNAL_H
#define CIS_ENGINE_USIGNAL_H

#include <functional>
#include <pthread.h>
#include <signal.h>

using namespace std;
namespace cis
{
typedef function<void(void)> signalFun;
class usignal
{
public:
  usignal(int flags, int sig, signalFun cb);
  ~usignal();

  void wait();

private:
  int sig__;
  struct sigaction sa__;
  signalFun cb__;
};
} // namespace cis

#endif
