#ifndef WOLF_FCONTEXT_H
#define WOLF_FCONTEXT_H

#include "base.h"
#include "util/memory.h"
#include <boost/context/detail/fcontext.hpp>
#include <functional>

#define STACK_MALLOC(sz) wolf::util::memory::malloc(sz)
#define STACK_FREE(p) wolf::util::memory::free(p)

NS_CC_O_BEGIN

using boost::context::detail::fcontext_t;
using boost::context::detail::jump_fcontext;
using boost::context::detail::make_fcontext;
using boost::context::detail::transfer_t;

// https://owent.net/2016/1270.html
// mprotect

typedef void (*fn_t)(transfer_t);

struct StackTraits {
  static int &getProtectStackPageSize();

  static bool protectStack(void *stack, size_t size, int pageSize);

  static void unprotectStack(void *stack, int pageSize);
};

NS_CC_O_END

#endif