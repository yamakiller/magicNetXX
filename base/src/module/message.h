#ifndef CIS_ENGINE_MODULE_MESSAGE_H
#define CIS_ENGINE_MODULE_MESSAGE_H

#include <boost/any.hpp>
#include <cstddef>
#include <iostream>
#include <vector>

namespace engine {
namespace module {

class message {
public:
  template <typename T>
  message(uint32_t source, const T &val) : m_source(source), m_value{val} {}

  template <typename T>
  message(uint32_t source, T &&val)
      : m_source(source), m_value{std::move(val)} {}

  message() = default;
  message(const message &) = delete;
  message(message &&) = default;
  message &operator=(const message &) = delete;
  message &operator=(message &&) = default;

  template <typename T> bool is() const noexcept {
    return typeid(T) == m_value.type();
  }

  template <typename T> auto get() { return boost::any_cast<T>(m_value); }

  template <typename T> const auto get() const {
    return boost::any_cast<T>(m_value);
  }

  template <typename T, typename U> message &match(U f) {
    if (is<T>()) {
      m_matched = true;
      f(get<T>());
    }

    return *this;
  }

  template <typename U> void otherwise(U f) {
    if (!m_matched)
      f();
  }

  template <typename T, typename U> void expect(U f) { f(get<T>()); }

private:
  uint32_t m_source;
  boost::any m_value;
  bool m_matched = false;
};

} // namespace module
} // namespace engine

#endif