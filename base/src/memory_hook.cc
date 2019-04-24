
#include "memory_hook.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define ATOM_CAS(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define ATOM_CAS_POINTER(ptr, oval, nval)                                      \
  __sync_bool_compare_and_swap(ptr, oval, nval)
#define ATOM_INC(ptr) __sync_add_and_fetch(ptr, 1)
#define ATOM_FINC(ptr) __sync_fetch_and_add(ptr, 1)
#define ATOM_DEC(ptr) __sync_sub_and_fetch(ptr, 1)
#define ATOM_FDEC(ptr) __sync_fetch_and_sub(ptr, 1)
#define ATOM_ADD(ptr, n) __sync_add_and_fetch(ptr, n)
#define ATOM_SUB(ptr, n) __sync_sub_and_fetch(ptr, n)
#define ATOM_AND(ptr, n) __sync_and_and_fetch(ptr, n)

#ifdef USE_JEMALLOC
#include <jemalloc/jemalloc.h>
#define rxmalloc je_malloc
#define rxcalloc je_calloc
#define rxrealloc je_realloc
#define rxfree je_free
#define rxmalloc_usable_size je_malloc_usable_size
#define rxmemalign je_memalign
#define rxaligned_alloc je_aligned_alloc
#define rxposix_memalign je_posix_memalign
#else
#define rxmalloc malloc
#define rxcalloc calloc
#define rxrealloc realloc
#define rxfree free
#ifdef __APPLE__
#include <malloc/malloc.h>
#define rxmalloc_useable_size malloc_size
#endif
//#define rxmemalign memalign
//#define rxaligned_alloc alligned_alloc
//#define rxposix_memalign posix_memalign
#endif

#ifdef __APPLE__
#define rxthread_slef ((uint32_t)(uintptr_t)pthread_self())
#else
#define rxthread_slef pthread_self()
#endif // __APPLE_

#define PREFIX_SIZE sizeof(struct engine::memory_hook::mem_cookie)

using namespace std;
namespace engine {
class memory_hook : noncopyable {
  friend class memory;

  struct mem_info {
    uint32_t thread_id;
    ssize_t memory_used;
  };

  struct mem_cookie {
    uint32_t thread_id;
  };

  static constexpr int MEMORY_BLACK_MAX = 0x10000;

private:
  static void malloc_error(size_t sz);
  static void fill_prefix(void *ptr);
  static void update_meminfo(uint32_t thread_id, size_t sz);
  static ssize_t *get_meminfo(uint32_t thread_id);

  static void clean_prefix(void *ptr);

private:
  static size_t __used_memory__;
  static size_t __used_block__;
  static mem_info __memory_blocks[MEMORY_BLACK_MAX];
};

size_t memory_hook::__used_memory__ = 0;
size_t memory_hook::__used_block__ = 0;
struct memory_hook::mem_info memory_hook::__memory_blocks[MEMORY_BLACK_MAX];

void memory_hook::malloc_error(size_t sz) {
  fprintf(stderr, "xmalloc: Out of memory trying to allocate %zu bytes\n", sz);
  fflush(stderr);
  abort();
}

void memory_hook::fill_prefix(void *ptr) {
  uint32_t thread_id = rxthread_slef;
  size_t size = rxmalloc_usable_size(ptr);
  struct mem_cookie *p =
      (struct mem_cookie *)((char *)ptr + size - sizeof(struct mem_cookie));
  p->thread_id = thread_id;
  update_meminfo(thread_id, size);
}

void memory_hook::update_meminfo(uint32_t thread_id, size_t sz) {
  ATOM_ADD(&__used_memory__, sz);
  ATOM_INC(&__used_block__);

  ssize_t *used_address = get_meminfo(thread_id);
  if (!used_address)
    return;
  ATOM_ADD(used_address, sz);
}

ssize_t *memory_hook::get_meminfo(uint32_t thread_id) {
  int hash = (int)(thread_id & (MEMORY_BLACK_MAX - 1));
  struct mem_info *info = &__memory_blocks[hash];
  uint32_t old_id = info->thread_id;
  ssize_t old_used = info->memory_used;
  if (old_id == 0 || old_used <= 0) {
    if (!ATOM_CAS(&info->thread_id, old_id, thread_id)) {
      return 0;
    }

    if (old_used < 0)
      ATOM_CAS(&info->memory_used, old_used, 0);
  }

  if (info->thread_id != thread_id) {
    return 0;
  }

  return &info->memory_used;
}

void memory_hook::clean_prefix(void *ptr) {
  size_t sz = rxmalloc_usable_size(ptr);
  struct mem_cookie *p = (struct mem_cookie *)((char *)ptr + sz - PREFIX_SIZE);
  uint32_t thread_id = p->thread_id;

  ATOM_SUB(&__used_memory__, sz);
  ATOM_DEC(&__used_block__);

  ssize_t *used_address = get_meminfo(thread_id);
  if (!used_address)
    return;
  ATOM_SUB(used_address, sz);
}

void *memory::malloc(size_t size) {
  void *ptr = rxmalloc(size + PREFIX_SIZE);
  if (!ptr)
    memory_hook::malloc_error(size);

  memory_hook::fill_prefix(ptr);
  return ptr;
}

void memory::free(void *ptr) {
  if (!ptr)
    return;
  memory_hook::clean_prefix(ptr);
  rxfree(ptr);
}

void *memory::calloc(size_t nmemb, size_t size) {
  void *ptr = rxcalloc(nmemb + ((PREFIX_SIZE + size - 1) / size), size);
  if (!ptr)
    memory_hook::malloc_error(size);
  memory_hook::fill_prefix(ptr);
  return ptr;
}

void *memory::realloc(void *ptr, size_t size) {
  if (!ptr)
    return memory::malloc(size);
  memory_hook::clean_prefix(ptr);
  void *newptr = rxrealloc(ptr, size + PREFIX_SIZE);
  if (!newptr)
    memory_hook::malloc_error(size);
  memory_hook::fill_prefix(newptr);
  return newptr;
}

char *memory::strdup(const char *str) {
  size_t strsz = strlen(str);
  char *dstr = (char *)memory::malloc(strsz + 1);
  memcpy(dstr, str, strsz);
  dstr[strsz] = '\0';
  return dstr;
}

void *memory::lalloc(void *ptr, size_t osize, size_t nsize) {
  if (nsize == 0) {
    rxfree(ptr);
    return NULL;
  } else {
    return rxrealloc(ptr, nsize);
  }
}

#ifdef USE_JEMALLOC

#ifdef __APPLE__
void *je_memalign(size_t alignment, size_t size) {
  void *mem = rxmalloc(size + (alignment - 1) + sizeof(void *));
  char *amem = ((char *)mem) + sizeof(void *);
  amem += alignment - ((uintptr_t)amem & (alignment - 1));
  ((void **)amem)[-1] = mem;
  return amem;
}
#endif

void *memory::memalign(size_t alignment, size_t size) {
  void *ptr = rxmemalign(alignment, size + PREFIX_SIZE);
  if (!ptr)
    memory_hook::malloc_error(size);
  memory_hook::fill_prefix(ptr);
  return ptr;
}

void *memory::aligned_alloc(size_t alignment, size_t size) {
  void *ptr =
      rxaligned_alloc(alignment, size + (size_t)((PREFIX_SIZE + alignment - 1) &
                                                 ~(alignment - 1)));
  if (!ptr)
    memory_hook::malloc_error(size);
  memory_hook::fill_prefix(ptr);
  return ptr;
}

int memory::posix_memalign(void **memptr, size_t alignment, size_t size) {
  int err = rxposix_memalign(memptr, alignment, size + PREFIX_SIZE);
  if (err)
    memory_hook::malloc_error(size);
  memory_hook::fill_prefix(*memptr);
  return err;
}
#endif

} // namespace engine