#ifndef CIS_ENGINE_BASE_H
#define CIS_ENGINE_BASE_H

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace engine
{
template <typename T>
using atomic_t = std::atomic<T>;

//写入debug打印
} // namespace engine

#endif