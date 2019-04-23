#ifndef CIS_ENGINE_CONTEXT_H
#define CIS_ENGINE_CONTEXT_H

#include "fcontext.h"

// https://github.com/yyzybb537/libgo/blob/master/libgo/context/context.h

namespace engine {
class context {
private:
  fcontext_t m_ctx;
  char *m_stack;
  uint32_t m_stacksize;
};
} // namespace engine

#endif
