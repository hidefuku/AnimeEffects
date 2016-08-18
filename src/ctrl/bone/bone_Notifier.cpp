#include "core/PoseKey.h"
#include "ctrl/bone/bone_Notifier.h"
#include "ctrl/bone/bone_GeoBuilder.h"

using namespace core;

namespace ctrl {
namespace bone {

Notifier::Notifier(
        core::Project& aProject,
        core::ObjectNode& aTarget,
        core::BoneKey& aKey,
        core::TimeLineEvent::Type aType)
    : mProject(aProject)
    , mTarget(aTarget)
    , mKey(aKey)
    , mEvent()
{
    mEvent.setType(aType);
    mEvent.pushTarget(mTarget, TimeKeyType_Bone,
                      mProject.animator().currentFrame().get());
}

void Notifier::notify(bool aIsUndo)
{
    // update pose keys
    for (auto child : mKey.children())
    {
        if (child->type() == core::TimeKeyType_Pose)
        {
            for (auto root : ((core::PoseKey*)child)->data().topBones())
            {
                XC_PTR_ASSERT(root);
                root->updateWorldTransform();
            }
        }
    }

    // create bone shape
    bone::GeoBuilder::build(mKey.data().topBones());

    // write bone influence map
    mKey.resetCaches(mProject, mTarget);

    // notify timeline modified
    mProject.onTimeLineModified(mEvent, aIsUndo);
}

} // namespace bone
} // namespace ctrl

