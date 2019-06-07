#ifndef WOLFE_ACTORWORKER_H
#define WOLFE_ACTORWORKER_H

#include "util/post.h"
#include "util/queue.h"
#include <condition_variable>
#include <mutex>

NS_CC_M_BEGIN

class actorWorker {
public:
  actorWorker();
  ~actorWorker();

  void doPost(uint32_t handle) {
    m_works.push(&handle);
    m_pid.doPost();
  }

  int32_t doStart();
  void doShutdown();

private:
  void doDispatch();

private:
  util::queue<uint32_t> m_works;
  util::post m_pid;
};

NS_CC_M_END

#endif