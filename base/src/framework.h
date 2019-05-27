#ifndef CIS_ENGINE_FRAMEWORK_H
#define CIS_ENGINE_FRAMEWORK_H
#include <stdint.h>
#include <stddef.h>

namespace engine
{

class icommandLine;
class framework
{
public:
  framework();
  virtual ~framework();
  static framework *instance();

  bool doInit(const icommandLine *opt);
  void doUnInit();
  void startLoop();

protected:
  virtual bool initialize(const icommandLine *opt) = 0;
  virtual bool loop() = 0;
  virtual void finalize() = 0;

private:
  void onSocketAccept(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketStart(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketData(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketClose(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketError(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketWarn(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
};
} // namespace engine

#endif