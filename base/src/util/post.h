#ifndef WOLF_POST_H
#define WOLF_POST_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace wolf {
namespace util {
class post final {
public:
  post();
  ~post();

  int32_t doStart(std::function<void()> func);
  inline bool isShutdown() { return m_shutdown; }
  void doShutdown();
  void doPost();

private:
  void defaultExecute();

private:
  uint32_t m_sleep;
  bool m_shutdown;

  std::function<void()> m_func;

  std::thread m_pid;
  std::mutex m_mtx;
  std::condition_variable m_cv;
};
} // namespace util
} // namespace wolf

#endif