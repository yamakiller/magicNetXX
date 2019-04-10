#ifndef CIS_ENGINE_IQ_H
#define CIS_ENGINE_IQ_H

#include "uspinlock.h"
//#include "umemory.h"
#include "uobject.h"
#include <string.h>

namespace cis
{
template <typename T, int DEFAULT_SIZE = 64, int EQ_OVERLOAD = 1024>
class iq : uobject
{
public:
  iq() : cap__(DEFAULT_SIZE),
         head__(0),
         tail__(0),
         inglobal__(1),
         overload__(0),
         overload_threshold__(EQ_OVERLOAD)
  {
    qs__ = (T *)umemory::malloc(sizeof(T) * cap__);
    memset(qs__, 0, sizeof(T) * cap__);
  }

  ~iq()
  {
    fprintf(stderr, "call ~iq()\n");
    T e;
    while (pop(&e))
    {
      local_drop(&e);
    }

    umemory::free(qs__);
    qs__ = NULL;
  }

  int length()
  {
    int head, tail, cap;
    k__.lock();
    head = head__;
    tail = tail__;
    cap = cap__;
    k__.unlock();

    if (head <= tail)
      return tail - head;
    return tail + cap - head;
  }

  void push(T *e)
  {
    assert(e);
    uspinlocking lk(&k__);

    qs__[tail__] = *e;
    if (++tail__ >= cap__)
      tail__ = 0;

    if (head__ == tail__)
      local_expand();
  }

  bool pop(T *e)
  {
    bool ret = false;

    uspinlocking lk(&k__);
    if (head__ != tail__)
    {
      ret = true;
      *e = qs__[head__++];
      if (head__ >= cap__)
        head__ = 0;

      int length = tail__ - head__;
      if (length < 0)
        length += cap__;

      while (length > overload_threshold__)
      {
        overload__ = length;
        overload_threshold__ *= 2;
      }
    }
    else
      overload_threshold__ = EQ_OVERLOAD;

    return ret;
  }

  int overload()
  {
    if (overload__)
    {
      int overload = overload__;
      overload__ = 0;
      return overload;
    }
    return 0;
  }

protected:
  void local_expand()
  {
    int i;
    T *new_qs = (T *)umemory::malloc(sizeof(T) * cap__ * 2);
    for (i = 0; i < cap__; i++)
    {
      memcpy((char *)&new_qs[i], (char *)&qs__[(head__ + i) % cap__], sizeof(T));
    }

    head__ = 0;
    tail__ = cap__;
    cap__ *= 2;

    umemory::free(qs__);
    qs__ = new_qs;
  }

  virtual void local_drop(T *e){};

protected:
  uspinlock k__;
  T *qs__;
  int cap__;
  int head__;
  int tail__;
  int inglobal__;
  int overload__;
  int overload_threshold__;
};
} // namespace cis

#endif
