#include "cmnd/BasicCommands.h"
#include "core/TimeKeyExpans.h"
#include "core/MeshKey.h"
#include "core/ImageKey.h"
#include "ctrl/ffd/ffd_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace ffd {

KeyOwner::KeyOwner()
    : key()
    , ownsKey()
    , parentKey()
{
}

KeyOwner::~KeyOwner()
{
    deleteOwnsKey();
}

void KeyOwner::createKey(
        const TimeLine& aLine, const LayerMesh& aAreaMesh,
        TimeKey* aAreaKey, int aFrame)
{
    // delete old holding key
    deleteOwnsKey();

    const TimeLine::MapType& map = aLine.map(TimeKeyType_FFD);

    if (map.contains(aFrame))
    {
        // a key is exists
        key = static_cast<FFDKey*>(map.value(aFrame));
        XC_PTR_ASSERT(key);
        ownsKey = false;

        // check parent mesh
        if (key->parent())
        {
            auto parentType = key->parent()->type();
            XC_ASSERT(parentType == TimeKeyType_Mesh || parentType == TimeKeyType_Image);
            parentKey = key->parent();
        }
    }
    else
    {
        // create new key
        const int vtxCount = aAreaMesh.vertexCount();
        XC_ASSERT(vtxCount > 0);
        key = new FFDKey();
        ownsKey = true;
        key->data().alloc(vtxCount);
        key->data().write(aLine.current().ffd().positions(), vtxCount);
        parentKey = aAreaKey;
    }
}

void KeyOwner::pushOwnsKey(cmnd::Stack& aStack, TimeLine& aLine, int aFrame)
{
    if (ownsKey)
    {
        aStack.push(new cmnd::GrabNewObject<FFDKey>(key));
        aStack.push(aLine.createPusher(TimeKeyType_FFD, aFrame, key));
        if (parentKey)
        {
            aStack.push(new cmnd::PushBackTree<TimeKey>(&parentKey->children(), key));
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
    parentKey = nullptr;
}

core::LayerMesh* KeyOwner::getParentMesh(core::ObjectNode*)
{
    if (parentKey)
    {
        if (parentKey->type() == TimeKeyType_Mesh)
        {
            return &(((MeshKey*)parentKey)->data());
        }
        else if (parentKey->type() == TimeKeyType_Image)
        {
            return &(((ImageKey*)parentKey)->data().gridMesh());
        }
        else
        {
            XC_ASSERT(0);
            return nullptr;
        }
    }
#if 0
    else if (aNode && aNode->timeLine())
    {
        auto imageKey = (ImageKey*)aNode->timeLine()->defaultKey(TimeKeyType_Image);
        if (imageKey) return &(imageKey->data().gridMesh());
    }
#endif
    return nullptr;
}

} // namespace ffd
} // namespace ctrl
