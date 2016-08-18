#ifndef CTRL_BONE_TARGET
#define CTRL_BONE_TARGET

#include "core/ObjectNode.h"

namespace ctrl {
namespace bone {

struct Target
{
    Target()
        : node()
        , mtx()
        , invMtx()
    {
    }

    explicit operator bool() const { return node; }
    core::ObjectNode* operator ->() { return node; }
    void clear() { node = nullptr; }

    core::ObjectNode* node;
    QMatrix4x4 mtx;
    QMatrix4x4 invMtx;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_TARGET

