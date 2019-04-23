#ifndef CIS_ENGINE_CLOCK_H
#define CIS_ENGINE_CLOCK_H

#include <chrono>
#include <limits>
#include <thread>
#include <time.h>

namespace engine {
class fastSteadyClock : public std::chrono::steady_clock {
public:
  static void ThreadRun() {}
};
} // namespace engine

#endif
