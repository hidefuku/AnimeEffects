#include "cmnd/BasicCommands.h"
#include "core/TimeKeyExpans.h"
#include "ctrl/ffd/ffd_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace ffd {

KeyOwner::KeyOwner()
    : key()
    , ownsKey()
    , worldMtx()
    , invMtx()
    , hasInv(false)
    , parentKey()
{
}

KeyOwner::~KeyOwner()
{
    deleteOwnsKey();
}

void KeyOwner::createKey(
        const TimeLine& aLine, const LayerMesh& aAreaMesh,
        MeshKey* aAreaKey, int aFrame)
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
            XC_ASSERT(key->parent()->type() == TimeKeyType_Mesh);
            parentKey = (MeshKey*)key->parent();
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

void KeyOwner::setupMtx(const core::ObjectNode& aNode, const core::TimeLine& aLine)
{
    const TimeKeyExpans& current = aLine.current();
    const QSizeF size(aNode.initialRect().size());
    const QVector3D center(size.width() * 0.5f, size.height() * 0.5f, 0.0f);

    worldMtx = current.srt().worldMatrix();
    worldMtx.translate(-center);
    invMtx = worldMtx.inverted(&hasInv);
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

core::LayerMesh* KeyOwner::getParentMesh(core::ObjectNode* node)
{
    if (parentKey)
    {
        return &parentKey->data();
    }
    if (node)
    {
        return node->gridMesh();
    }
    return nullptr;
}

} // namespace ffd
} // namespace ctrl
