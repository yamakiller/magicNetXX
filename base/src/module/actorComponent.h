#ifndef CIS_ENGINE_MODULE_ACTORCOMPONENT_H
#define CIS_ENGINE_MODULE_ACTORCOMPONENT_H

namespace engine
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
  virtual int doInit(actor *parent, void *parm);

protected:
  actor *m_parent;
};
} // namespace module
} // namespace engine
#endif