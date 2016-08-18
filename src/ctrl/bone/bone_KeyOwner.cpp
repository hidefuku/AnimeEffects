#include "cmnd/BasicCommands.h"
#include "ctrl/bone/bone_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace bone {

void KeyOwner::pushOwnsKey(cmnd::Stack& aStack, TimeLine& aLine, int aFrame)
{
    if (ownsKey)
    {
        aStack.push(new cmnd::GrabNewObject<BoneKey>(key));
        aStack.push(aLine.createPusher(TimeKeyType_Bone, aFrame, key));
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
}

} // namespace bone
} // namespace ctrl
