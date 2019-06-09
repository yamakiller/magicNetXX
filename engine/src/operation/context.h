#ifndef WOLFXT_H
#define WOLFXT_H

#include "fcontext.h"
#include "util/mobject.h"

NS_CC_O_BEGIN

class context : public util::mobject {
  typedef void (*fnc_t)(void *data);

public:
  context(fnc_t fn, void *ud, size_t stackSize)
      : m_fn(fn), m_ud(ud), m_isIn(false), m_stackSize(stackSize) {
    m_stack = (char *)STACK_MALLOC(m_stackSize);
    m_ctx =
        make_fcontext(m_stack + m_stackSize, m_stackSize, [](transfer_t trans) {
          context *ctx = static_cast<context *>(trans.data);
          *(&ctx->getTlsContext()) = trans.fctx;
          ctx->m_fn(ctx->m_ud);
        });

    int protectPage = StackTraits::getProtectStackPageSize();
    if (protectPage &&
        StackTraits::protectStack(m_stack, m_stackSize, protectPage))
      m_protectPage = protectPage;
  }

  ~context() {
    if (m_stack) {
      if (m_protectPage)
        StackTraits::unprotectStack(m_stack, m_protectPage);

      STACK_FREE(m_stack);
      m_stack = NULL;
    }
  }

  inline void SwapIn() {
    transfer_t t = jump_fcontext(m_ctx, (void *)this);
    m_ctx = t.fctx;
  }

  inline void SwapTo(context &other) {
    transfer_t t = jump_fcontext(other.m_ctx, other.m_ud);
    m_ctx = t.fctx;
  }

  inline void SwapOut() {
    transfer_t t = jump_fcontext(getTlsContext(), 0);
    *(&getTlsContext()) = t.fctx;
  }

  fcontext_t &getTlsContext() {
    static thread_local fcontext_t tls_context;
    return tls_context;
  }

private:
  fcontext_t m_ctx;
  char *m_stack;
  fnc_t m_fn;
  void *m_ud;
  bool m_isIn;
  uint32_t m_stackSize;
  int m_protectPage;
};

NS_CC_O_END

#endif
