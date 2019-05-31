#ifndef WOLF_MODULE_ACTORCOMPONENT_H
#define WOLF_MODULE_ACTORCOMPONENT_H

#include "actor.h"
#include "actorSystem.h"
#include "message.h"
#include "operation/worker.h"
#include "util/mobject.h"
#include <boost/any.hpp>
#include <functional>
#include <stdint.h>
#include <unordered_map>
#include <vector>

namespace wolf {
namespace module {

typedef void (*dispatchFunc)(void *, int32_t, uint32_t, boost::any);
typedef boost::any (*packFunc)(void *, ...);
typedef boost::any (*unpackFunc)(void *, void *, uint32_t);

struct packData {
  void *_data;
  size_t _size;
};

struct messageProtocol {
  int8_t _msgId;

  dispatchFunc _dispatch;

  packFunc _pack;

  unpackFunc _unpack;
};

struct coEntery {
  uint64_t _id;
  operation::worker::suspendEntry _entry;
  std::function<void(void)> _func;
};

typedef std::vector<messageProtocol> msgProtoTable;
typedef operation::worker::suspendEntry suspendEntery;
typedef std::unordered_map<int32_t, coEntery> suspedTable;
typedef std::unordered_map<uint64_t, int32_t> sleepTable;

class actorComponent : public util::mobject {
  friend class actor;

public:
  actorComponent();
  virtual ~actorComponent();

public:
  virtual int32_t doInit(actor *parent, void *parm);
  virtual int32_t doRun(struct message *msg);

  void doRegisterProtocol(messageProtocol proto);

  uintptr_t getHandle() { return m_parent->handle(); }

protected:
  template <typename... Args>
  void doSend(int32_t msgId, uint32_t dst, int32_t session, Args... parm);

  void doTimeout(int tm, std::function<void(void)> func);

  void doWait(struct coEntery co);

  bool doWakeup(struct coEntery co);

  void doExit();

private:
  void quit();

  int32_t getSuspedSize() {
    return m_suspedSession.size() + m_suspedSleep.size();
  }

  int32_t genSession();

  bool suspendSleep(int32_t session, struct coEntery co);

  struct coEntery *getSusped(int32_t session);
  bool insertSusped(int32_t session, struct coEntery co);
  void removeSusped(int32_t session);

  int32_t getSleepSusped(struct coEntery co);
  void insertSleepSusped(int32_t session, struct coEntery co);
  void removeSleepSusped(uint64_t enteryId);

  messageProtocol *getProtocol(int32_t msgId);

  int32_t dispatchMessage(struct message *msg);
  void unknownResponse(int32_t session, uint32_t source, void *msg,
                       uint32_t sz);
  void unknownRequest(int32_t msgId, int32_t session, uint32_t source,
                      void *msg, uint32_t sz);

  static void staticErrorDispatch(void *param, int32_t session, uint32_t src,
                                  boost::any data);
  void errorDispatch(int32_t errorSession, uint32_t errorSrc, boost::any &data);

  static boost::any staticErrorUnPack(void *param, void *data, uint32_t size);

protected:
  actor *m_parent;
  int32_t m_session;
  msgProtoTable m_proto;
  suspedTable m_suspedSession;
  sleepTable m_suspedSleep;
};

template <typename... Args>
void actorComponent::doSend(int32_t msgId, uint32_t dst, int32_t session,
                            Args... parm) {
  messageProtocol *proto = getProtocol(msgId);
  if (proto == nullptr) {
    SYSLOG_ERROR(m_parent->handle(),
                 "Failed to send data, protocol is not defined({})", msgId);
    return;
  }

  if (proto->_pack == nullptr) {
    SYSLOG_ERROR(m_parent->handle(),
                 "Failed to send data, protocol binding function is not "
                 "defined(pack:{})",
                 msgId);
    return;
  }

  packData pk = boost::any_cast<packData>(proto->_pack(this, parm...));

  if (INST(actorSystem, doSendMessage, m_parent->handle(), dst, msgId, session,
           pk._data, pk._size) != 0) {
    util::memory::free(pk._data); // TODO: 是否是必须
    SYSLOG_ERROR(m_parent->handle(), "Data transmission failed({})", msgId);
    return;
  }
}

} // namespace module
} // namespace wolf
#endif