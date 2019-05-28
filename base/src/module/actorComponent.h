#ifndef WOLF_MODULE_ACTORCOMPONENT_H
#define WOLF_MODULE_ACTORCOMPONENT_H

#include "util/mobject.h"
#include <stdint.h>

namespace wolf
{
namespace module
{
class actor;
class actorComponent : public util::mobject
{
public:
  actorComponent();
  virtual ~actorComponent();

public:
  virtual int32_t doInit(actor *parent, void *parm);
  virtual int32_t doRun(struct message *msg) = 0;

protected:
  actor *m_parent;
};
} // namespace module
} // namespace wolf
#endif