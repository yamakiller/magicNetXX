#include "iocpEpoll.h"

#ifdef UT_PLATFORM_LINUX

#include <sys/epoll.h>
#include <sys/errno.h>
#include <unistd.h>

namespace engine {
namespace network {
iocpEpoll::iocpEpoll() { m_handle = epoll_create(1024); }
iocpEpoll::~iocpEpoll() {
  ::close(m_handle);
  m_handle = -1;
}

bool iocpEpoll::doRegister(wsocket_t sock, void *ud) {
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.ptr = ud;
  if (epoll_ctl(m_handle, EPOLL_CTL_ADD, sock, &ev) == -1)
    return false;
  return true;
}

void iocpEpoll::doUnRegister(wsocket_t sock) {
  epoll_ctl(m_handle, EPOLL_CTL_DEL, sock, NULL);
}

void iocpEpoll::doToWrite(wsocket_t sock, void *ud, bool enable) {
  struct epoll_event ev;
  ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
  ev.data.ptr = ud;
  epoll_ctl(m_handle, EPOLL_CTL_MOD, sock, &ev);
}

int iocpEpoll::onWait() {
  struct epoll_event ev[IOCP_EVENT_WAIT_MAX];
  int n = epoll_wait(m_handle, ev, IOCP_EVENT_WAIT_MAX, -1);
  if (n <= 0) {
    if (errno == EINTR)
      return -2;
    return -1;
  }

  int i;
  for (i = 0; i < n; i++) {
    m_evts[i].s = ev[i].data.ptr;
    unsigned flag = ev[i].events;
    m_evts[i].iswrite = (flag & EPOLLOUT) != 0;
    m_evts[i].isread = (flag & (EPOLLIN | EPOLLHUP)) != 0;
    m_evts[i].iserror = (flag & EPOLLERR) != 0;
    m_evts[i].iseof = false;
  }
  return n;
}

} // namespace network
} // namespace engine

#endif