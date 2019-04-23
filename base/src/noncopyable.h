#ifndef CIS_ENGINE_NONCOPYABLE_H
#define CIS_ENGINE_NONCOPYABLE_H

namespace engine {

class noncopyable {
protected:
  noncopyable() {}
  ~noncopyable() {}

private:
  noncopyable(const noncopyable &);
  noncopyable &operator=(const noncopyable &);
};

} // namespace engine

#endif