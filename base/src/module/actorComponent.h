#ifndef CIS_ENGINE_MODULE_ACTORCOMPONENT_H
#define CIS_ENGINE_MODULE_ACTORCOMPONENT_H

#include <stdint.h>

namespace wolf
{
namespace module
{
class actor;
class actorComponent
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