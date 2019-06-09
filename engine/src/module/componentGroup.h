#ifndef WOLF_COMPONENT_GROUP_H
#define WOLF_COMPONENT_GROUP_H

#include "util/spinlock.h"
#include <string>

#define MAX_COMPONENT_TYPE 64

NS_CC_M_BEGIN

typedef void *(*component_dl_create)(void);
typedef void (*component_dl_release)(void *inst);
typedef void (*component_dl_signal)(void *inst, int signal);

struct component {
  const char *_name;
  void *_dll;
  component_dl_create _create;
  component_dl_release _release;
  component_dl_signal _signal;
};

class componentGroup {
public:
  componentGroup();
  virtual ~componentGroup();

public:
  int32_t doInit(const char *path);
  void doDestory();
  struct component *getComponent(const char *name);

private:
  struct component *get(const char *name);
  void *tryOpen(const char *name);
  void *getApi(struct component *comp, const char *apiName);
  bool openSym(struct component *comp);

private:
  int32_t m_count;
  std::string m_path;
  component m_compts[MAX_COMPONENT_TYPE];
  util::spinlock m_mutex;
};

NS_CC_M_END

#endif