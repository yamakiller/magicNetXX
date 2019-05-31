#ifndef WOLF_API_SOCKET_H
#define WOLF_APIAPI_SOCKET_H

#include "module/actorComponent.h"
#include "util/noncopyable.h"
#include "util/spinlock.h"
#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#define SOCKET_BACKLOG 32
// 2 ** 12 == 4096
#define SOCKET_LARGE_PAGE_NODE 12
#define SOCKET_BUFFER_LIMIT (256 * 1024)

namespace wolf {
namespace api {
struct wfBufferNode {
  const char *_data;
  uint32_t _sz;
  wfBufferNode *_next;
};

struct wfSocketBuffer {
  uint32_t _size;
  uint32_t _offset;
  struct wfBufferNode *_head;
  struct wfBufferNode *_tail;
};

class wfSocket {
public:
  int32_t _id;
  bool _connected;
  boost::any _connecting;
  boost::any _readRequired;
  wfSocketBuffer _buffer;
  int32_t _bufferLimit;
  boost::any _co;
  boost::any _closing;
  std::weak_ptr<module::actor> _opaque;
  std::function<void(int32_t clientId, const char *addr)> _callback;

public:
  int32_t doRead(char *outBUffer, int outLen);
  boost::any doReadStrLine(bool check, std::string sep);
  std::string doReadStrAll();

private:
  std::string popString(int sz, int skip);
  int32_t popBuffer(char *outBuffer, int outLen);
};

typedef std::shared_ptr<wfSocket> ptrSocket;
typedef std::unordered_map<int32_t, ptrSocket> socket_maps;
typedef std::vector<wfBufferNode *> buffer_pools;

class socketApi : public util::noncopyable {
public:
  static void doRequire(module::actorComponent *cpt);

  static int32_t doListen(std::shared_ptr<module::actor> ptr, const char *addr,
                          int prot, int backlog);

  static boost::any doConnection(std::shared_ptr<module::actor> ptr,
                                 const char *addr, int port);

  static boost::any
  doStart(std::shared_ptr<module::actor> ptr, int32_t id,
          std::function<void(int32_t clientId, const char *addr)> func);

  static void doClose(std::shared_ptr<module::actor> ptr, int32_t id);

  static void doSetLimit(int32_t id, int limit);

  static bool doBlock(int32_t id);

  static int32_t doGetDataSize(int32_t id);
  static int32_t doRead(int32_t id, char *outBuffer, int outLen);
  static std::string doReadLine(int32_t id, bool *ok, std::string sep);
  static std::string doReadAll(int32_t id);
  static bool doDisconnected(int32_t id);
  static void doShutdown(int32_t id);

public:
  static void staticSocketDispatch(void *param, int32_t session, uint32_t src,
                                   boost::any data);
  static boost::any staticSocketUnPack(void *param, void *data, uint32_t size);

  static void onAccept(uintptr_t opaque, int32_t id, int32_t clientId,
                       char *addr);
  static void onData(uintptr_t opaque, int32_t id, uint32_t size, char *data);
  static void onStart(int32_t id, char *addr);
  static void onError(uintptr_t opaque, int32_t id, char *err);
  static void onWarning(int32_t id, int size);
  static void onUdp(uintptr_t opaque, int32_t id, int size, char *data,
                    char *addr);
  static void onClose(int32_t id);

  static ptrSocket getSocket(int32_t id);
  static void pushSocket(int32_t id, ptrSocket);
  static void removeSocket(int32_t id);
  static void doSuspend(ptrSocket s);
  static void doWakeup(ptrSocket s);

  static boost::any
  createSocket(std::shared_ptr<module::actor> ptr, int32_t id,
               std::function<void(int32_t clientId, const char *addr)> func);

  static uint32_t pushBuffer(wfSocketBuffer *ptrBuffer, char *data,
                             uint32_t sz);

  static void freeBuffer(wfSocketBuffer *ptrBuffer);

  static void freeBufferNode(wfSocketBuffer *ptrBuffer);

  static void pushBufferNode(wfBufferNode *ptrNode);
  static wfBufferNode *popBufferPool();
  static wfBufferNode *newBufferPool(int32_t num);

private:
  static socket_maps m_socketPool;
  static util::spinlock m_socketMutex;

  static buffer_pools m_bufferPool;
  static util::spinlock m_bufferMutex;
};
} // namespace api
} // namespace wolf
#endif