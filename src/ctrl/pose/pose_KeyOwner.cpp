#include "cmnd/BasicCommands.h"
#include "ctrl/pose/pose_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace pose {

void KeyOwner::pushOwnsKey(cmnd::Stack& aStack, TimeLine& aLine, int aFrame)
{
    if (ownsKey)
    {
        aStack.push(new cmnd::GrabNewObject<PoseKey>(key));
        aStack.push(aLine.createPusher(TimeKeyType_Pose, aFrame, key));
        if (parent)
        {
            aStack.push(new cmnd::PushBackTree<TimeKey>(&parent->children(), key));
        }
        ownsKey = false;
    }
}

void KeyOwner::deleteOwnsKey()
{
    if (key && ownsKey)
    {
        delete key;
    }
    ownsKey = false;
    key = nullptr;
    parent = nullptr;
}

} // namespace pose
} // namespace ctrl
