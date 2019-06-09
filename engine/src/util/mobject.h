#ifndef WOLF_MOBJECT_H
#define WOLF_MOBJECT_H

#include "memory.h"

NS_CC_U_BEGIN

class mobject {
public:
  mobject() = default;
  virtual ~mobject() = default;

public:
  void *operator new(size_t size) { return memory::malloc(size); }

  void *operator new[](size_t size) { return memory::malloc(size); }

  void operator delete(void *p) { memory::free(p); }

  void operator delete[](void *p) { memory::free(p); }
};

NS_CC_U_END

#endif