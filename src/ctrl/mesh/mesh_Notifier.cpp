#include "ctrl/mesh/mesh_Notifier.h"

using namespace core;

namespace ctrl {
namespace mesh {

Notifier::Notifier(
        core::Project& aProject,
        core::ObjectNode& aTarget,
        core::MeshKey& aKey,
        core::TimeLineEvent::Type aType)
    : mProject(aProject)
    , mTarget(aTarget)
    , mKey(aKey)
    , mEvent()
{
    mEvent.setType(aType);
    mEvent.pushTarget(mTarget, TimeKeyType_Mesh,
                      mProject.animator().currentFrame().get());
}

void Notifier::notify(bool aIsUndo)
{
    // setup rendering attributes
    mKey.updateGLAttribute();

    // notify timeline modified
    mProject.onTimeLineModified(mEvent, aIsUndo);
}

} // namespace mesh
} // namespace ctrl

