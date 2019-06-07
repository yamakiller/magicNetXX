#include "iocpKqueue.h"

#ifdef UT_PLATFORM_APPLE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <unistd.h>

NS_CC_N_BEGIN

iocpKqueue::iocpKqueue() { m_handle = kqueue(); }
iocpKqueue::~iocpKqueue() {
  close(m_handle);
  m_handle = -1;
}

bool iocpKqueue::doRegister(wsocket_t sock, void *ud) {
  struct kevent ke;
  EV_SET(&ke, sock, EVFILT_READ, EV_ADD, 0, 0, ud);
  if (kevent(m_handle, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR) {
    return false;
  }
  EV_SET(&ke, sock, EVFILT_WRITE, EV_ADD, 0, 0, ud);
  if (kevent(m_handle, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR) {
    EV_SET(&ke, sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(m_handle, &ke, 1, NULL, 0, NULL);
    return false;
  }
  EV_SET(&ke, sock, EVFILT_WRITE, EV_DISABLE, 0, 0, ud);
  if (kevent(m_handle, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR) {
    doUnRegister(sock);
    return false;
  }
  return true;
}

void iocpKqueue::doUnRegister(wsocket_t sock) {
  struct kevent ke;
  EV_SET(&ke, sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  kevent(m_handle, &ke, 1, NULL, 0, NULL);
  EV_SET(&ke, sock, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
  kevent(m_handle, &ke, 1, NULL, 0, NULL);
}

void iocpKqueue::doToWrite(wsocket_t sock, void *ud, bool enable) {
  struct kevent ke;
  EV_SET(&ke, sock, EVFILT_WRITE, enable ? EV_ENABLE : EV_DISABLE, 0, 0, ud);
  if (kevent(m_handle, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR) {
  }
}

int iocpKqueue::onWait() {
  struct kevent ev[IOCP_EVENT_WAIT_MAX];
  int n = kevent(m_handle, NULL, 0, ev, IOCP_EVENT_WAIT_MAX, NULL);
  if (n <= 0) {
    if (errno == EINTR)
      return -2;
    return -1;
  }

  int i;
  for (i = 0; i < n; i++) {
    m_evts[i].s = ev[i].udata;
    unsigned filter = ev[i].filter;
    bool eof = (ev[i].flags & EV_EOF) != 0;
    m_evts[i].iswrite = (filter == EVFILT_WRITE) && (!eof);
    m_evts[i].isread = (filter == EVFILT_READ) && (!eof);
    m_evts[i].iserror = (ev[i].flags & EV_ERROR) != 0;
    m_evts[i].iseof = eof;
  }

  return n;
}

NS_CC_N_END

#endif