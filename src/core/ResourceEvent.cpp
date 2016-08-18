#include "core/ResourceEvent.h"

namespace core
{

ResourceEvent::ResourceEvent(Project& aProject)
    : mProject(aProject)
    , mRoot()
    , mTargets()
{
}

} // namespace core

