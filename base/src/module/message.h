#ifndef WOLF_MODULE_MESSAGE_H
#define WOLF_MODULE_MESSAGE_H

#include <cstddef>
#include <iostream>
#include <vector>
#include <string.h>

#define MESSAGE_EXT_MAX 32
#define MESSAGE_EXT_ID_BYTES sizeof(uint8_t)
#define MESSAGE_EXT_SZ_BYTES MESSAGE_EXT_MAX - sizeof(uint8_t)
#define MESSAGE_EXT_SZ_MASK 0xFFFFF

namespace wolf
{
namespace module
{
enum messageId
{
  M_ID_ERROR = 1,
  M_ID_TIMEOUT,
  M_ID_RESOUE,
  M_ID_TEXT,
  M_ID_SOCKET,
  M_ID_QUIT,
  M_ID_MAX,
};

struct message
{
  uint32_t _src;
  uint32_t _dst;
  int32_t _session;
  void *_data;
  char _ext[MESSAGE_EXT_MAX];
};

class messageApi
{
public:
  static inline struct message getMessage(uint8_t msgId, uint32_t src, uint32_t dst, int32_t session = 0, void *data = nullptr, size_t sz = 0);
  static inline int getMessageId(struct message *msg);
  static inline uint32_t getMessageSize(struct message *msg);
};

struct message messageApi::getMessage(uint8_t msgId, uint32_t src, uint32_t dst, int32_t session, void *data, size_t sz)
{
  struct message msg;
  msg._src = src;
  msg._dst = dst;
  msg._session = session;
  msg._data = data;
  uint32_t tmpsize = (sz & MESSAGE_EXT_SZ_MASK);
  tmpsize <<= MESSAGE_EXT_ID_BYTES;
  memcpy(&msg._ext[0], &msgId, MESSAGE_EXT_ID_BYTES);
  memcpy(((char *)&msg._ext[0]) + MESSAGE_EXT_ID_BYTES, &tmpsize, MESSAGE_EXT_SZ_BYTES);

  return msg;
}

int messageApi::getMessageId(struct message *msg)
{
  uint8_t msgId;
  memcpy(&msgId, &msg->_ext[0], sizeof(uint8_t));
  return (int)msgId;
}

uint32_t messageApi::getMessageSize(struct message *msg)
{
  uint32_t msgSize = 0;
  memcpy(&msgSize, ((char *)&msg->_ext[0]) + MESSAGE_EXT_ID_BYTES, MESSAGE_EXT_SZ_BYTES);
  msgSize >>= MESSAGE_EXT_ID_BYTES;
  return msgSize;
}

} // namespace module
} // namespace wolf

#endif