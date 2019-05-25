#ifndef CIS_ENGINE_UTIL_MUTEXS_H
#define CIS_ENGINE_UTIL_MUTEXS_H

#include <condition_variable>
#include <mutex>

namespace engine {
namespace util {
class wrMutex {
public:
  wrMutex() = default;
  ~wrMutex() = default;

  void lock_read() {
    std::unique_lock<std::mutex> ulk(m_counterMutex);
    m_condRead.wait(ulk, [=]() -> bool { return m_writeCount == 0; });
    ++m_readCount;
  }
  void lock_write() {
    std::unique_lock<std::mutex> ulk(m_counterMutex);
    ++m_writeCount;
    m_condWrite.wait(
        ulk, [=]() -> bool { return m_readCount == 0 && !m_inWriteFlag; });
    m_inWriteFlag = true;
  }
  void release_read() {
    std::unique_lock<std::mutex> ulk(m_counterMutex);
    if (--m_readCount == 0 && m_writeCount > 0) {
      m_condWrite.notify_one();
    }
  }
  void release_write() {
    std::unique_lock<std::mutex> ulk(m_counterMutex);
    if (--m_writeCount == 0) {
      m_condRead.notify_all();
    } else {
      m_condWrite.notify_one();
    }
    m_inWriteFlag = false;
  }

private:
  volatile size_t m_readCount{0};
  volatile size_t m_writeCount{0};
  volatile bool m_inWriteFlag{false};
  std::mutex m_counterMutex;
  std::condition_variable m_condWrite;
  std::condition_variable m_condRead;
};

template <typename _RWMutex> class unique_writeguard {
public:
  explicit unique_writeguard(_RWMutex &rwlockable) : m_rwlockable(rwlockable) {
    m_rwlockable.lock_write();
  }
  ~unique_writeguard() { m_rwlockable.release_write(); }

private:
  unique_writeguard() = delete;
  unique_writeguard(const unique_writeguard &) = delete;
  unique_writeguard &operator=(const unique_writeguard &) = delete;

private:
  _RWMutex &m_rwlockable;
};

template <typename _RWMutex> class unique_readguard {
public:
  explicit unique_readguard(_RWMutex &rwlockable) : m_rwlockable(rwlockable) {
    m_rwlockable.lock_read();
  }
  ~unique_readguard() { m_rwlockable.release_read(); }

private:
  unique_readguard() = delete;
  unique_readguard(const unique_readguard &) = delete;
  unique_readguard &operator=(const unique_readguard &) = delete;

private:
  _RWMutex &m_rwlockable;
};

} // namespace util
} // namespace engine

#endif