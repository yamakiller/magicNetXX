

#include "socketChannel.h"
#include "socketSystem.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

NS_CC_N_BEGIN

socketChannel::socketChannel() {

  m_recvCtrl = CHANNEL_INVALID;
  m_sendCtrl = CHANNEL_INVALID;
  m_controlCtrl = CHANNEL_INVALID;

  FD_ZERO(&m_rsfds);
  int fd[2];
  if (pipe(fd)) {
    fprintf(stderr, "ctrl: create socket pair failed.\n");
    assert(nullptr);
    return;
  }

  if (!INSTGET_VAR(socketSystem, m_iocp)->doRegister(fd[0], nullptr)) {
    ::close(fd[0]);
    ::close(fd[1]);
    assert(nullptr);
    return;
  }

  m_recvCtrl = fd[0];
  m_sendCtrl = fd[1];
  m_controlCtrl = 1;

  assert(m_recvCtrl < FD_SETSIZE);
}

socketChannel::~socketChannel() {
  doCloseCtrl(&m_recvCtrl);
  doCloseCtrl(&m_sendCtrl);
  m_controlCtrl = CHANNEL_INVALID;
}

bool socketChannel::isInvalid() {
  return m_recvCtrl != CHANNEL_INVALID && m_sendCtrl != CHANNEL_INVALID &&
         m_controlCtrl != CHANNEL_INVALID;
}

int socketChannel::getError() { return m_error; }

void socketChannel::doSend(struct requestPacket *request, char type, int len) {
  request->header[0] = (uint8_t)type;
  request->header[1] = (uint8_t)len;
  doWrite((const char *)&request->header[0], len + 2);
}

void socketChannel::doWrite(const char *data, const size_t bytes) {
  for (;;) {
    ssize_t n = ::write(m_sendCtrl, data, bytes);
    if (n < 0) {
      if (errno != EINTR) {
        fprintf(stderr, "Socket Channel : send command error %s.\n",
                strerror(errno));
      }
      continue;
    }
    assert((const size_t)n == bytes);
    return;
  }
}

void socketChannel::doRecv(void *buffer, const size_t bytes) {
  for (;;) {
    int n = ::read(m_recvCtrl, buffer, bytes);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      fprintf(stderr, "Socket Channel : read pipe error %s.\n",
              strerror(errno));
      return;
    }
    assert((const size_t)n == bytes);
    return;
  }
}

void socketChannel::doRest() { m_controlCtrl = 1; }

void socketChannel::doCloseCtrl(int *ctrl) {
  if (*ctrl != CHANNEL_INVALID) {
    INSTGET_VAR(socketSystem, m_iocp)->doUnRegister(*ctrl);
    close(*ctrl);
    *ctrl = CHANNEL_INVALID;
  }
}

bool socketChannel::isRecv() {
  struct timeval tv = {0, 0};
  int retval;

  FD_SET(m_recvCtrl, &m_rsfds);
  retval = ::select(m_recvCtrl + 1, &m_rsfds, NULL, NULL, &tv);
  if (retval == 1)
    return true;
  return false;
}

int socketChannel::doWait(int32_t idx, int32_t n) {
  if (!m_controlCtrl) {
    m_error = errCode::CH_NOEINTR;
    return -1;
  }

  if (!isRecv()) {
    m_controlCtrl = 0;
    m_error = errCode::CH_NOEINTR;
    return -1;
  }

  return doProccess(idx, n);
}

int socketChannel::doProccess(int32_t idx, int32_t n) {
  /* data Piece */
  uint8_t data[256];
  /* [0 bit command][ 1 bit bytes] */
  uint8_t header[2];

  doRecv(header, sizeof(header));
  int command = header[0];
  int bytes = header[1];
  int32_t handle;
  int ret;
  doRecv(data, bytes);

  switch ((char)command) {
  case 'D':
  case 'P': {
    struct requestSend *request = (struct requestSend *)data;
    handle = request->_handle;
    ret = m_sendFunc(request, nullptr);
    break;
  }
  case 'S': {
    struct requestStart *request = (struct requestStart *)data;
    handle = request->_handle;
    ret = m_startFunc(request);
    break;
  }
  case 'K': {
    struct requestClose *request = (struct requestClose *)data;
    handle = request->_handle;
    ret = m_closeFunc(request);
    break;
  }
  case 'C': {
    struct requestConnect *request = (struct requestConnect *)data;
    handle = request->_handle;
    ret = m_connectFunc(request);
    break;
  }
  case 'T':
    ret = m_setoptFunc((struct requestSetopt *)data);
    m_error = errCode::CH_EINTR;
    break;
  case 'X': {
    m_error = errCode::CH_EXIT;
    return -1;
  }
  default: {
    fprintf(stderr, "Socket Channel: Unknown command %c.\n", command);
    ret = -1;
    break;
  }
  }

  if (ret == (int32_t)socketMessageType::M_SOCKET_CLOSE ||
      ret == (int32_t)socketMessageType::M_SOCKET_ERROR) {
    m_error == errCode::CH_ERROR;
    m_clearClosedFunc(handle, idx, n);
  } else {
    m_error = errCode::CH_EINTR;
  }
  return -1;
}

NS_CC_N_END