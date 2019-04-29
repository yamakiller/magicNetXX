#ifndef CIS_ENGINE_CONTEXT_H
#define CIS_ENGINE_CONTEXT_H

#include "fcontext.h"
// https://github.com/yyzybb537/libgo/blob/master/libgo/context/context.h

namespace engine
{
class context
{
public:
  context(fn_t fn, void *ud, size_t stackSize)
      : m_fn(fn), m_ud(ud), m_stackSize(stackSize)
  {
    m_stack = (char *)STACK_MALLOC(m_stackSize);
    m_ctx = make_fcontext(m_stack + m_stackSize, m_stackSize, m_fn);

    int protectPage = StackTraits::getProtectStackPageSize();
    if (protectPage && StackTraits::protectStack(m_stack, m_stackSize, protectPage))
      m_protectPage = protectPage;
  }

  ~context()
  {
    if (m_stack)
    {
      if (m_protectPage)
        StackTraits::unprotectStack(m_stack, m_protectPage);

      STACK_FREE(m_stack);
      m_stack = NULL;
    }
  }

  inline void SwapIn()
  {
    transfer_t t = jump_fcontext(m_ctx, m_ud);
    *(&getTlsContext()) = t.fctx;
  }

  inline void SwapTo(context &other)
  {
    transfer_t t = jump_fcontext(other.m_ctx, other.m_ud);
    m_ctx = t.fctx;
  }

  inline void SwapOut()
  {
    transfer_t t = jump_fcontext(getTlsContext(), 0);
    m_ctx = t.fctx;
  }

  fcontext_t &getTlsContext()
  {
    static thread_local fcontext_t tls_context;
    return tls_context;
  }

private:
  fcontext_t m_ctx;
  char *m_stack;
  fn_t m_fn;
  void *m_ud;
  uint32_t m_stackSize;
  int m_protectPage;
};
} // namespace engine

#endif
