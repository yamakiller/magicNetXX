#include "clock.h"
#include "timestamp.h"

namespace engine {
clock::clock() {
  uint32_t tmp_cur = 0;
  timestamp::getSystemTime(&m_starttime, &tmp_cur);
  m_current = tmp_cur;
  m_current_point = timestamp::getTime();
}

clock::~clock() {}

void clock::tick() {
  uint64_t ct = timestamp::getTime();
  if (ct < m_current_point) {

    m_current_point = ct;
  } else if (ct != m_current_point) {
    uint32_t diff = (uint32_t)(ct - m_current_point);
    m_current_point = ct;
    m_current += diff;
  }
}
} // namespace engine