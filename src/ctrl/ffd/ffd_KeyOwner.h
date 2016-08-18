#ifndef CTRL_FFD_KEYOWNER_H
#define CTRL_FFD_KEYOWNER_H

#include <QMatrix4x4>
#include "cmnd/Stack.h"
#include "core/ObjectNode.h"
#include "core/TimeLine.h"
#include "core/LayerMesh.h"
#include "core/MeshKey.h"
#include "core/FFDKey.h"

namespace ctrl {
namespace ffd {

struct KeyOwner
{
    KeyOwner();
    ~KeyOwner();

    explicit operator bool() const { return key; }
    bool owns() const { return ownsKey; }

    void createKey(
            const core::TimeLine& aLine,
            const core::LayerMesh& aAreaMesh,
            core::MeshKey* aAreaKey,
            int aFrame);
    void setupMtx(const core::ObjectNode& aNode, const core::TimeLine& aLine);

    void pushOwnsKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);
    void deleteOwnsKey();

    core::LayerMesh* getParentMesh(core::ObjectNode* node);

    core::FFDKey* key;
    bool ownsKey;

    QMatrix4x4 worldMtx;
    QMatrix4x4 invMtx;
    bool hasInv;

    core::MeshKey* parentKey;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_KEYOWNER_H
