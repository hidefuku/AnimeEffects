#include "cmnd/BasicCommands.h"
#include "ctrl/mesh/mesh_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace mesh {

void KeyOwner::pushOwnsKey(cmnd::Stack& aStack, TimeLine& aLine, int aFrame)
{
    if (ownsKey)
    {
        aStack.push(new cmnd::GrabNewObject<MeshKey>(key));
        aStack.push(aLine.createPusher(TimeKeyType_Mesh, aFrame, key));
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

} // namespace mesh
} // namespace ctrl
