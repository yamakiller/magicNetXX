#ifndef WOLF_MODULE_ACTORCOMPONENT_H
#define WOLF_MODULE_ACTORCOMPONENT_H

#include "util/mobject.h"
#include "operation/worker.h"
#include "message.h"
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>
#include "actorSystem.h"

namespace wolf
{
namespace module
{
class actor;

typedef void (*dispatchFunc)(int32_t, uint32_t, boost::any &);
typedef boost::any &(*packFunc)(...);
typedef boost::any &(*unpackFunc)(void *, uint32_t);

struct packData
{
  void *_data;
  size_t _size;
};

struct messageProtocol
{
  int8_t _msgId;

  dispatchFunc _dispatch;

  packFunc _pack;

  unpackFunc _unpack;
};

struct co
{
  operation::worker::suspendEntry _entry;
  std::function<void(void)> _func;
};

typedef std::vector<messageProtocol> msgProtoTable;
typedef operation::worker::suspendEntry suspendEntery;
typedef std::unordered_map<int32_t, co> suspedTable;

class actorComponent : public util::mobject
{
public:
  actorComponent();
  virtual ~actorComponent();

public:
  virtual int32_t doInit(actor *parent, void *parm);
  virtual int32_t doRun(struct message *msg);

protected:
  template <typename... Args>
  void doSend(int32_t msgId, uint32_t dst, int32_t session, Args... parm);

private:
  co *getSusped(int32_t session);
  void removeSusped(int32_t session);

  messageProtocol *getProtocol(int32_t msgId);
  void registerProtocol(messageProtocol proto);

  int32_t dispatchMessage(struct message *msg);
  void unknownResponse(int32_t session, uint32_t source, void *msg, uint32_t sz);
  void unknownRequest(int32_t msgId, int32_t session, uint32_t source, void *msg, uint32_t sz);

protected:
  actor *m_parent;
  msgProtoTable m_proto;
  suspedTable m_susped;
};

template <typename... Args>
void actorComponent::doSend(int32_t msgId, uint32_t dst, int32_t session, Args... parm)
{
  messageProtocol *proto = getProtocol(msgId);
  if (proto == nullptr)
  {
    SYSLOG_ERROR(m_parent->handle(), "Failed to send data, protocol is not defined({})", msgId);
    return;
  }

  if (proto->_pack == nullptr)
  {
    SYSLOG_ERROR(m_parent->handle(), "Failed to send data, protocol binding function is not defined(pack:{})", msgId);
    return;
  }

  packData pk = boost::any_cast<packData>(proto->_pack(parm...));

  if (INST(actorSystem, doSendMessage, m_parent->handle(), dst, msgId, session, pk._data, pk._size) != 0)
  {
    util::memory::free(pk._data); //TODO: 是否是必须
    SYSLOG_ERROR(m_parent->handle(), "Data transmission failed({})", msgId);
    return;
  }
}

} // namespace module
} // namespace wolf
#endif