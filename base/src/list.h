#ifndef CIS_ENGINE_D_LIST_H
#define CIS_ENGINE_D_LIST_H

#include <queue>

namespace engine {
template <class T> class list {
public:
  list() {}
  ~list() {}

  void push(T val) { m_tb.push(val); }

  T pop() {
    T result = m_tb.front();
    m_tb.pop();
    return result;
  }

  list<T> *cut(int32_t n) {
    list<T> *result = new list<T>();
    for (int i = 0; i < n && !m_tb.empty(); i++) {
      result->push(this->pop());
    }
    return result;
  }

  const bool empty() { return m_tb.empty(); }

  const size_t size() { return m_tb.size(); }

private:
  std::queue<T> m_tb;
};

} // namespace engine

#endif