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

namespace wolf
{
namespace api
{

typedef std::weak_ptr<module::actor> aWeakPtr;
typedef std::shared_ptr<module::actor> aSharedPtr;
typedef std::function<void(int32_t, const char *)> socketfunc;

struct stringResult
{
  bool _ok;
  std::string _str;
};

class socketApi : public util::noncopyable
{
public:
  static void doRequire(module::actorComponent *cpt);

  static int32_t doListen(aSharedPtr ptr, const char *addr, int port, int backlog);
  static boost::any doConnection(aSharedPtr ptr, const char *addr, int port);
  static boost::any doStart(aSharedPtr ptr, int32_t id, socketfunc func);
  static void doClose(aSharedPtr ptr, int32_t id);
  static void doSetLimit(int32_t id, int limit);
  static bool doBlock(int32_t id);
  static int32_t doGetDataSize(int32_t id);
  static int32_t doSend(int32_t id, char *data, uint32_t len);
  static int32_t doRead(int32_t id, char *outBuffer, int outLen);
  static boost::any doReadLine(int32_t id, std::string sep);
  static std::string doReadAll(int32_t id);
  static bool doDisconnected(int32_t id);
  static void doShutdown(int32_t id);
};

} // namespace api
} // namespace wolf
#endif