#include "fcontext.h"

#ifdef __linux__
#include <sys/mman.h>
#endif

namespace wolf {
namespace operation {
int &StackTraits::getProtectStackPageSize() {
  static int size = 0;
  return size;
}

#ifdef __linux__
bool StackTraits::protectStack(void *stack, size_t size, int pageSize) {
  if (!pageSize)
    return false;

  if ((int)size <= getpagesize() * (pageSize + 1))
    return false;

  void *protect_page_addr =
      ((std::size_t)stack & 0xfff)
          ? (void *)(((size_t)stack & ~(size_t)0xfff) + 0x1000)
          : stack;
  if (-1 == mprotect(protect_page_addr, getpagesize() * pageSize, PROT_NONE)) {

    return false;
  } else {

    return true;
  }
}
void StackTraits::unprotectStack(void *stack, int pageSize) {
  if (!pageSize)
    return;

  void *protect_page_addr =
      ((std::size_t)stack & 0xfff)
          ? (void *)(((std::size_t)stack & ~(std::size_t)0xfff) + 0x1000)
          : stack;
  if (-1 == mprotect(protect_page_addr, getpagesize() * pageSize,
                     PROT_READ | PROT_WRITE)) {
  } else {
  }
}
#else
bool StackTraits::protectStack(void *stack, size_t size, int pageSize) {
  return false;
}
void StackTraits::unprotectStack(void *stack, int pageSize) { return; }
#endif

} // namespace operation
} // namespace wolf