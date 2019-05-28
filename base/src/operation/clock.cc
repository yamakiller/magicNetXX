#include "clock.h"
#include "util/timestamp.h"

namespace wolf {
namespace operation {
clockSlot::~clockSlot() {}

void clockSlot::local_push(std::shared_ptr<struct clockHandle> node_ptr) {
  m_lst.push_back(node_ptr);
}

std::shared_ptr<struct clockHandle> clockSlot::local_pop() {
  std::shared_ptr<struct clockHandle> result_ptr = nullptr;
  if (m_lst.empty()) {
    return nullptr;
  }

  result_ptr = m_lst.front();
  m_lst.pop_front();
  return result_ptr;
}

void clockSlot::local_dispatch(
    std::shared_ptr<struct clockHandle> current_ptr) {
  if (!current_ptr->_cancel) {
    void *param = (void *)current_ptr->_data;
    current_ptr->_func(param);
  }
}

void clockSlot::local_clear() { m_lst.clear(); }

clock::clock() : m_time(0), m_timeId{1} {
  uint32_t tmp_cur = 0;
  util::timestamp::getSystemTime(&m_starttime, &tmp_cur);
  m_current = tmp_cur;
  m_current_point = util::timestamp::getTime();
}

clock::~clock() {
  int i;
  m_cLock.lock();

  for (i = 0; i < CLOCK_SLOT; i++) {
    m_slots[i].local_clear();
  }

  for (i = 0; i < CLOCK_LEVEL; i++) {
    m_ts[0][i].local_clear();
    m_ts[1][i].local_clear();
    m_ts[2][i].local_clear();
    m_ts[3][i].local_clear();
  }

  m_cLock.unlock();
}

clock::timeEntery clock::timeOut(uint32_t time, clockFun const &func,
                                 const void *parm) {
  if (time <= 0) {

    func((void *)parm);
    uint64_t tmp_id = ++m_timeId;
    return timeEntery{std::shared_ptr<struct clockHandle>(nullptr), tmp_id};

  } else {
    struct clockHandle *lnde = new clockHandle();
    assert(lnde);

    uint64_t tmp_id = ++m_timeId;
    m_cLock.lock();
    lnde->_id = tmp_id;
    lnde->_expire = time + m_time;
    lnde->_cancel = 0;
    lnde->_func = func;
    lnde->_data = parm;
    std::shared_ptr<struct clockHandle> tmp_ptr(lnde);
    local_append(tmp_ptr);
    m_cLock.unlock();

    return timeEntery{tmp_ptr, tmp_id};
  }
}

void clock::timeCancel(timeEntery &entery) {
  auto tm = entery._tm.lock();
  if (!tm) {
    return;
  }
  tm->_cancel = 1;
}

void clock::tick() {
  uint64_t ct = util::timestamp::getTime();
  if (ct < m_current_point) {
    m_current_point = ct;
  } else if (ct != m_current_point) {
    uint32_t diff = (uint32_t)(ct - m_current_point);
    m_current_point = ct;
    m_current += diff;

    for (uint32_t i = 0; i < diff; i++)
      update();
  }
}

} // namespace operation
} // namespace wolf