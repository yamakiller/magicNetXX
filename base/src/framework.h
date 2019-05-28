#ifndef CIS_ENGINE_FRAMEWORK_H
#define CIS_ENGINE_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>
#include "commandLineOption.h"

static const int ENGINE_MAJOR_VERSION = 1;
static const int ENGINE_MINOR_VERSION = 0;
static const int ENGINE_PATCH_VERSION = 0;

#define ENGINE_MAJOR_VERSION 1
#define ENGINE_MINOR_VERSION 0
#define ENGINE_PATCH_VERSION 0

namespace wolf
{

class framework
{
public:
  framework();
  virtual ~framework();
  static framework *instance();

  bool doInit(const commandLineOption *opt);
  void doUnInit();
  void startLoop();

protected:
  virtual bool initialize(const commandLineOption *opt) = 0;
  virtual bool loop() = 0;
  virtual void finalize() = 0;

  /*private:  //需要，放到SystemSocket中
  void onSocketAccept(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketStart(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketData(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketClose(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketError(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);
  void onSocketWarn(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t sz);*/
};
} // namespace wolf

#endif