#ifndef CIS_ENGINE_BASE_H
#define CIS_ENGINE_BASE_H

#include <functional>
#include <stdint.h>
#include <memory>
#include <atomic>
#include <algorithm>
#include <string.h>

namespace engine {
template <typename T>
using atomic_t = std::atomic<T>;
}

#endif