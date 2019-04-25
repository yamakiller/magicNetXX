#ifndef CIS_ENGINE_LOCKFREERING_QUEUE_H
#define CIS_ENGINE_LOCKFREERING_QUEUE_H

#include <atomic>
#include <limits>
#include <stdint.h>

namespace engine {

struct lockFreeResult {
  bool success = false;
  bool notify = false;
};

// 无锁环形队列
template <typename T, typename SizeType = size_t> class lockFreeRingQueue {
public:
  typedef SizeType uint_t;
  typedef std::atomic<uint_t> atomic_t;

  // 多申请一个typename T的空间, 便于判断full和empty.
  explicit lockFreeRingQueue(uint_t capacity)
      : m_capacity(reCapacity(capacity)), m_readable{0}, m_write{0}, m_read{0},
        m_writable{uint_t(m_capacity - 1)} {
    m_buffer = (T *)malloc(sizeof(T) * m_capacity);
  }

  ~lockFreeRingQueue() {
    // destory elements.
    uint_t read = consume(m_read);
    uint_t readable = consume(m_readable);
    for (; read < readable; ++read) {
      m_buffer[read].~T();
    }

    free(m_buffer);
  }

  template <typename U> lockFreeResult Push(U &&t) {
    lockFreeResult result;

    // 1.m_write步进1.
    uint_t write, writable;
    do {
      write = relaxed(m_write);
      writable = consume(m_writable);
      if (write == writable)
        return result;

    } while (!m_write.compare_exchange_weak(write, mod(write + 1),
                                           std::memory_order_acq_rel,
                                           std::memory_order_relaxed));

    // 2.数据写入
    new (m_buffer + write) T(std::forward<U>(t));

    // 3.更新readable
    uint_t readable;
    do {
      readable = relaxed(m_readable);
    } while (!m_readable.compare_exchange_weak(write, mod(readable + 1),
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed));

    // 4.检查写入时是否empty
    result.notify = (write == mod(writable + 1));
    result.success = true;
    return result;
  }

  lockFreeResult Pop(T &t) {
    lockFreeResult result;

    // 1.m_read步进1.
    uint_t read, readable;
    do {
      read = relaxed(m_read);
      readable = consume(m_readable);
      if (read == readable)
        return result;

    } while (!m_read.compare_exchange_weak(read, mod(read + 1),
                                          std::memory_order_acq_rel,
                                          std::memory_order_relaxed));

    // 2.读数据
    t = std::move(m_buffer[read]);
    m_buffer[read].~T();

    // 3.更新writable
    // update condition: mod(m_writable + 1) == m_read
    //               as: m_writable == mod(m_read + m_capacity - 1)
    uint_t check = mod(read + m_capacity - 1);
    while (!m_writable.compare_exchange_weak(
        check, read, std::memory_order_acq_rel, std::memory_order_relaxed));

    // 4.检查读取时是否full
    result.notify = (read == mod(m_readable + 1));
    result.success = true;
    return result;
  }

private:
  inline uint_t relaxed(atomic_t &val) {
    return val.load(std::memory_order_relaxed);
  }

  inline uint_t acquire(atomic_t &val) {
    return val.load(std::memory_order_acquire);
  }

  inline uint_t consume(atomic_t &val) {
    return val.load(std::memory_order_consume);
  }

  inline uint_t mod(uint_t val) { return val % m_capacity; }

  inline size_t reCapacity(uint_t capacity) { return (size_t)capacity + 1; }

private:
  size_t m_capacity;
  T *m_buffer;

  // [m_write, m_writable] 可写区间, m_write == m_writable is full.
  // read后更新writable
  atomic_t m_write;
  atomic_t m_writable;

  // [m_read, m_readable) 可读区间, m_read == m_readable is empty.
  // write后更新readable
  atomic_t m_read;
  atomic_t m_readable;
};

} // namespace engine

#endif