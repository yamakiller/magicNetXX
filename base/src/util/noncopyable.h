#ifndef CIS_ENGINE_UTIL_NONCOPYABLE_H
#define CIS_ENGINE_UTIL_NONCOPYABLE_H

namespace wolf {
namespace util {
class noncopyable {
protected:
  noncopyable() {}
  ~noncopyable() {}

private:
  noncopyable(const noncopyable &);
  noncopyable &operator=(const noncopyable &);
};
} // namespace util
} // namespace wolf

#endif