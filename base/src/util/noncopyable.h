#ifndef WOLF_UTIL_NONCOPYABLE_H
#define WOLF_UTIL_NONCOPYABLE_H
#include "platform.h"

NS_CC_U_BEGIN

class noncopyable {
protected:
  noncopyable() {}
  ~noncopyable() {}

private:
  noncopyable(const noncopyable &);
  noncopyable &operator=(const noncopyable &);
};

NS_CC_U_END

#endif