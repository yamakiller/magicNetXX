#include "post.h"

namespace wolf {
namespace util {

post::post() : m_sleep(0), m_shutdown(false) {}

post::~post() {}

int32_t post::doStart(std::function<void()> func) {
  m_func = func;
  std::thread t([this]() { this->defaultExecute(); });
  m_pid.swap(t);
  return 0;
}

void post::doShutdown() {
  {
    std::unique_lock<std::mutex> lck(m_mtx);
    m_shutdown = true;
    m_cv.notify_one();
  }

  if (m_pid.joinable()) {
    m_pid.join();
  }
}

void post::doPost() {
  std::unique_lock<std::mutex> lck(m_mtx);
  if (m_sleep > 0) {
    m_cv.notify_one();
  }
}

void post::defaultExecute() {
  while (!m_shutdown) {
    {
      std::unique_lock<std::mutex> lck(m_mtx);
      ++m_sleep;
      if (!m_shutdown) {
        m_cv.wait(lck);
      }
      --m_sleep;
    }

    if (!m_shutdown) {
      m_func();
    }
  }
}

} // namespace util
} // namespace wolf