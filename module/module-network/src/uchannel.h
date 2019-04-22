#ifndef MODULE_UNET_UCHANNEL_H
#define MODULE_UNET_UCHANNEL_H

#include "usocket.h"
#include <stdint.h>
#include <urecursivelock.h>
#include <uspinlock.h>

namespace cis {
namespace network {

enum u_channel_s {
  UNET_C_INVALID = 0,
  UNET_C_USED = 1,
  UNET_C_CONNECTING = 2,
  UNET_C_CONNECTED = 3,
  UNET_C_HALFCLOSE = 4,
};

struct uforwardb {
  uint16_t offset;
  uint16_t size;
  char *data;
};

class uforward_list {
  struct uforward_node {
    uforwardb *db;
    struct uforward_node *next;
  };

public:
  uforward_list();
  ~uforward_list();

  int32_t empty();
  int32_t size();

  void push(struct uforwardb *d);
  struct uforwardb *pop();

private:
  int32_t m_override;
};

class uchannel {
  struct uchannel_info {
    uint64_t forward_tts;
    uint64_t forward_bytes;
    uint64_t recv_tts;
    uint64_t recv_bytes;
  };

public:
  uchannel();
  ~uchannel();

public:
  uint32_t id;

protected:
  u_sock_t m_sock;
  int32_t m_status;
  int32_t m_addr;
  int32_t m_port;

  uforward_list m_forward;
  uchannel_info m_info;

  int32_t m_remote_addr;
  int32_t m_remote_port;

  uspinlock m_lock;
};

} // namespace network
} // namespace cis
#endif
