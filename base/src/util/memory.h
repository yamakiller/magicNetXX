#ifndef CIS_ENGINE_MODULE_MEMORY_H
#define CIS_ENGINE_MODULE_MEMORY_H

#include "noncopyable.h"
#include <cstddef>

namespace wolf {
namespace util {
class memory : public noncopyable {
public:
  static void *malloc(size_t size);
  static void *calloc(size_t nmemb, size_t size);
  static void *realloc(void *ptr, size_t size);
  static char *strdup(const char *str);
  static void *lalloc(void *ptr, size_t osize, size_t nsize);
#ifdef USE_JEMALLOC
  static void *memalign(size_t alignment, size_t size);
  static void *aligned_alloc(size_t alignment, size_t size);
  static int posix_memalign(void **memptr, size_t alignment, size_t size);
#endif
  static void free(void *ptr);
};
} // namespace util
} // namespace wolf

#endif