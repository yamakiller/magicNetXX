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
  int m_iSig;
  struct sigaction m_sa;
  signalFun m_callback;
};
} // namespace cis

#endif
