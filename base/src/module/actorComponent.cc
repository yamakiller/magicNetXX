#include "actorComponent.h"

namespace wolf
{
namespace module
{
actorComponent::actorComponent() : m_parent(nullptr)
{
}

actorComponent::~actorComponent()
{
}

int32_t actorComponent::doInit(actor *parent, void *parm)
{
    m_parent = parent;
    return 0;
}

} // namespace module
} // namespace wolf