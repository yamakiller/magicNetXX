#include "actorComponent.h"

namespace engine
{
namespace module
{
actorComponent::actorComponent() : m_parent(nullptr)
{
}

actorComponent::~actorComponent()
{
}

int actorComponent::doInit(actor *parent, void *parm)
{
    m_parent = parent;
    return 0;
}

} // namespace module
} // namespace engine