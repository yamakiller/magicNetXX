#ifndef CIS_ENGINE_CLOCK_H
#define CIS_ENGINE_CLOCK_H

#include "singleton.h"
#include "spinlock.h"
#include <chrono>
#include <limits>
#include <thread>
#include <time.h>

namespace engine
{
class clock : public singleton<clock>
{
public:
  clock();
  ~clock();

  void tick();

  uint64_t now() { return m_current; }

  uint32_t startTime() { return m_starttime; }

private:
  uint32_t m_starttime;
  uint64_t m_current;
  uint64_t m_current_point;
};
} // namespace engine

#endif
