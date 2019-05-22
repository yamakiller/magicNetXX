#ifndef CIS_ENGINE_CLOCK_H
#define CIS_ENGINE_CLOCK_H

#include "base.h"
#include "singleton.h"
#include "spinlock.h"
#include <chrono>
#include <functional>
#include <limits>
#include <list>
#include <stdint.h>
#include <thread>
#include <time.h>

#define CLOCK_SLOT_SHOFT 8
#define CLOCK_SLOT (1 << CLOCK_SLOT_SHOFT)
#define CLOCK_SLOT_MASK (CLOCK_SLOT - 1)
#define CLOCK_LEVEL_SHOFT 6
#define CLOCK_LEVEL (1 << CLOCK_LEVEL_SHOFT)
#define CLOCK_LEVEL_MASK (CLOCK_LEVEL - 1)

namespace engine {

typedef std::function<void(void *)> clockFun;

struct clockHandle {
  uint64_t _id;
  uint32_t _expire;
  uint32_t _cancel;
  clockFun _func;
  const void *_data;

  void *operator new(size_t size) { return en_malloc(size); }
  void operator delete(void *p) { en_free(p); }
};

class clockSlot {
  friend class clock;

public:
  clockSlot() noexcept {};
  ~clockSlot();

private:
  void local_push(std::shared_ptr<struct clockHandle> node_ptr);
  std::shared_ptr<struct clockHandle> local_pop();

  void local_dispatch(std::shared_ptr<struct clockHandle> current_ptr);

  inline void local_process(spinlock *plock) {
    while (!m_lst.empty()) {
      std::shared_ptr<struct clockHandle> curPtr = m_lst.front();
      m_lst.pop_front();
      plock->unlock();
      local_dispatch(curPtr);
      plock->lock();
    }
  }

  void local_clear();

private:
  std::list<std::shared_ptr<struct clockHandle>> m_lst;
};

class clock : public singleton<clock> {
public:
  struct timeEntery {
    std::weak_ptr<clockHandle> _tm;
    uint64_t _id;
  };

public:
  clock();
  ~clock();

  void tick();

  uint64_t now() { return m_current; }

  uint32_t startTime() { return m_starttime; }

  timeEntery timeOut(uint32_t time, clockFun const &func, const void *parm);

  void timeCancel(timeEntery &entery);

private:
  inline void local_shift() {
    int mask = CLOCK_SLOT;
    uint32_t ct = ++m_time;
    if (ct == 0)
      local_move(3, 0);
    else {
      uint32_t time = ct >> CLOCK_SLOT_SHOFT;
      int i = 0;
      while ((ct & (mask - 1)) == 0) {
        int idx = time & CLOCK_LEVEL_MASK;
        if (idx != 0) {
          local_move(i, idx);
          break;
        }
        mask <<= CLOCK_LEVEL_SHOFT;
        time >>= CLOCK_LEVEL_SHOFT;
        ++i;
      }
    }
  }

  inline void local_execute() {
    int idx = m_time & CLOCK_SLOT_MASK;
    m_slots[idx].local_process(&m_cLock);
  }

  inline void update() {
    m_cLock.lock();
    local_execute();
    local_shift();
    local_execute();
    m_cLock.unlock();
  }

  inline void local_move(int level, int idx) {
    std::shared_ptr<struct clockHandle> curr_ptr = nullptr;

    while ((curr_ptr = m_ts[level][idx].local_pop()) != NULL) {
      local_append(curr_ptr);
    }
  }

  inline void local_append(std::shared_ptr<struct clockHandle> lnde) {
    uint32_t time = lnde->_expire;
    uint32_t current_time = m_time;
    if ((time | (CLOCK_SLOT)) == (current_time | CLOCK_SLOT_MASK))

      m_slots[time & CLOCK_LEVEL_MASK].local_push(lnde);
    else {
      int i;
      uint32_t mask = CLOCK_SLOT << CLOCK_LEVEL_SHOFT;
      for (i = 0; i < 3; i++) {
        if ((time | (mask - 1)) == (current_time | (mask - 1)))
          break;
        mask <<= CLOCK_LEVEL_SHOFT;
      }
      m_ts[i][(time >> (CLOCK_SLOT_SHOFT + i * CLOCK_LEVEL_SHOFT)) &
              CLOCK_LEVEL_MASK]
          .local_push(lnde);
    }
  }

private:
  clockSlot m_slots[CLOCK_SLOT];
  clockSlot m_ts[4][CLOCK_LEVEL];
  spinlock m_cLock;

  uint32_t m_time;
  uint32_t m_starttime;
  uint64_t m_current;
  uint64_t m_current_point;

  atomic_t<long> m_timeId;
};
} // namespace engine

#endif
