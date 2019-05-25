#include "memory.h"
#include "base.h"
#include <jemalloc/jemalloc.h>

#define rxmalloc je_malloc
#define rxcalloc je_calloc
#define rxrealloc je_realloc
#define rxfree je_free
#define rxmalloc_usable_size je_malloc_usable_size
#define rxmemalign je_memalign
#define rxaligned_alloc je_aligned_alloc
#define rxposix_memalign je_posix_memalig

namespace engine {
namespace util {

class memory_hook : public noncopyable {
  friend class memory;

private:
  static void mallocError(size_t sz);
  static void fillPrefix(void *ptr);
  static void cleanPrefix(void *ptr);

private:
  static atomic_t<size_t> m_usedMemory;
  static atomic_t<size_t> m_usedBlocks;
};

atomic_t<size_t> memory_hook::m_usedMemory = {0};
atomic_t<size_t> memory_hook::m_usedBlocks = {0};

void memory_hook::mallocError(size_t sz) {
  fprintf(stderr, "xmalloc: Out of memory trying to allocate %zu bytes\n", sz);
  fflush(stderr);
  abort();
}

void memory_hook::fillPrefix(void *ptr) {
  size_t size = rxmalloc_usable_size(ptr);
  // m_usedMemory += sz;
  ++m_usedBlocks;
}

void memory_hook::cleanPrefix(void *ptr) {
  size_t sz = rxmalloc_usable_size(ptr);
  // m_usedMemory -= sz;
  --m_usedBlocks;
}

void *memory::malloc(size_t size) {
  void *ptr = rxmalloc(size);
  if (!ptr) {
    memory_hook::mallocError(size);
  }

  memory_hook::fillPrefix(ptr);
  return ptr;
}

void memory::free(void *ptr) {
  if (!ptr) {
    return;
  }

  memory_hook::cleanPrefix(ptr);
  rxfree(ptr);
}

void *memory::calloc(size_t nmemb, size_t size) {
  void *ptr = rxcalloc(nmemb, size);
  if (!ptr) {
    memory_hook::mallocError(size);
  }

  memory_hook::fillPrefix(ptr);
  return ptr;
}

void *memory::realloc(void *ptr, size_t size) {
  if (!ptr) {
    return memory::malloc(size);
  }

  memory_hook::cleanPrefix(ptr);
  void *newptr = rxrealloc(ptr, size);
  if (!newptr) {
    memory_hook::mallocError(size);
  }

  memory_hook::fillPrefix(newptr);
  return newptr;
}

void *memory::lalloc(void *ptr, size_t osize, size_t nsize) {
  if (nsize == 0) {
    rxfree(ptr);
    return NULL;
  } else {
    return rxrealloc(ptr, nsize);
  }
}

} // namespace util
} // namespace engine