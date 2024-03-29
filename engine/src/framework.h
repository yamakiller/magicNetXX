#ifndef WOLF_FRAMEWORK_H
#define WOLF_FRAMEWORK_H

#include "commandLineOption.h"
#include "platform.h"
#include <stddef.h>
#include <stdint.h>

static const int WOLF_MAJOR_VERSION = 1;
static const int WOLF_MINOR_VERSION = 0;
static const int WOLF_PATCH_VERSION = 0;

#define WOLF_MAJOR_VERSION 1
#define WOLF_MINOR_VERSION 0
#define WOLF_PATCH_VERSION 0

NS_CC_BEGIN

class framework {
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
  void onSocketAccept(uintptr_t opaque, int32_t handle, int32_t ud, void *data,
  size_t sz); void onSocketStart(uintptr_t opaque, int32_t handle, int32_t ud,
  void *data, size_t sz); void onSocketData(uintptr_t opaque, int32_t handle,
  int32_t ud, void *data, size_t sz); void onSocketClose(uintptr_t opaque,
  int32_t handle, int32_t ud, void *data, size_t sz); void
  onSocketError(uintptr_t opaque, int32_t handle, int32_t ud, void *data, size_t
  sz); void onSocketWarn(uintptr_t opaque, int32_t handle, int32_t ud, void
  *data, size_t sz);*/
};

NS_CC_END

#endif