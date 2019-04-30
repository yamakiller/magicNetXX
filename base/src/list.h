#ifndef CIS_ENGINE_D_LIST_H
#define CIS_ENGINE_D_LIST_H

#include <queue>

namespace engine {
template <class T> class dlist {
public:
  dlist() {}
  ~dlist() {}

  void push(T &val) { m_tb.push(val); }

  T pop() {
    T result = m_tb.front();
    m_tb.pop();
    return result;
  }

  dlist<T> *cut(int32_t n) {
    dlist<T> *result = new dlist<T>();
    for (int i = 0; i < n && !m_tb.empty(); i++) {
      result->push(this->pop());
    }
    return result;
  }

  const bool empty() { return m_tb.empty(); }

private:
  std::queue<T> m_tb;
};

} // namespace engine

#endif