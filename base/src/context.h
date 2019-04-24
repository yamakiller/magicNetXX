#ifndef CIS_ENGINE_CONTEXT_H
#define CIS_ENGINE_CONTEXT_H

#include "fcontext.h"

// https://github.com/yyzybb537/libgo/blob/master/libgo/context/context.h

namespace engine {
class context {

  context(fn_t fn, intptr_t ud, size_t stackSize)
      : m_fn(fn), m_ud(ud), m_stackSize(stackSize) {
    m_stack = (char *)::malloc(m_stackSize);
    m_ctx = make_fcontext(m_stack + m_stackSize, m_stackSize, m_fn);
  }

  ~context() {
    if (m_stack) {
      ::free(m_stack);
      m_stack = NULL;
    }
  }

  inline void SwapIn() { jump_fcontext(&getTlsContext(), m_ctx, m_ud); }

  inline void SwapTo(context &other) {
    jump_fcontext(&m_ctx, other.m_ctx, other.m_ud);
  }

  inline void SwapOut() { jump_fcontext(&m_ctx, getTlsContext(), 0); }

  fcontext_t &getTlsContext() {
    static thread_local fcontext_t tls_context;
    return tls_context;
  }

private:
  fcontext_t m_ctx;
  char *m_stack;
  fn_t m_fn;
  intptr_t m_ud;
  uint32_t m_stackSize;
};
} // namespace engine

#endif
