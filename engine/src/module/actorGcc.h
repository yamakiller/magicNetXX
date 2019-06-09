#ifndef WOLF_MODULE_ACTOR_GCC
#define WOLF_MODULE_ACTOR_GCC

#include "util/queue.h"
#include <thread>

NS_CC_M_BEGIN

class actorGcc {
public:
  actorGcc();
  ~actorGcc() = default;

  int32_t doStart();
  void doShutdown();
  void enterGcc(uint32_t actorId);
  void doLoop();

private:
  bool m_shutdown;
  util::queue<uint32_t> m_qGcc;
  std::thread m_pid;
};

NS_CC_M_END

#endif