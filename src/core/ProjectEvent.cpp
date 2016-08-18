#include "core/ProjectEvent.h"

namespace core
{

ProjectEvent::ProjectEvent(Project& aProject, Type aType)
    : mProject(aProject)
    , mType(aType)
{
}

} // namespace core
