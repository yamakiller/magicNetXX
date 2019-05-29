#ifndef WOLF_MODULE_ACTORCOMPONENT_H
#define WOLF_MODULE_ACTORCOMPONENT_H

#include "util/mobject.h"
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>

namespace wolf
{
namespace module
{
class actor;

struct messageProtocol
{
  int8_t _msgId;

  std::function<void(int32_t, uint32_t, boost::any &data)> _dispatch;

  std::function<boost::any &(void *, uint32_t)> _unpack;
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
} // namespace module
} // namespace wolf
#endif