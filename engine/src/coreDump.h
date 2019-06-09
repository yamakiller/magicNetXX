#ifndef WOLF_COREDUMP_H
#define WOLF_COREDUMP_H

#include "base.h"
#include "util/singleton.h"

NS_CC_BEGIN

class coreDump : public util::singleton<coreDump> {
public:
  coreDump() = default;
  virtual ~coreDump() = default;

public:
  void doListen();

private:
  void outStack(int sig);

private:
  static void coreSignal(int signal);
};

NS_CC_END

#endif