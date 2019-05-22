#ifndef CIS_ENGINE_BASE_H
#define CIS_ENGINE_BASE_H

#include "format.h"
#include <algorithm>
#include <assert.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <jemalloc/jemalloc.h>

#define en_malloc(sz) je_malloc(sz)
#define en_free(ptr) je_free(ptr)

namespace engine {
template <typename T> using atomic_t = std::atomic<T>;

extern std::mutex gDbgLock;
//写入debug打印
} // namespace engine

#endif