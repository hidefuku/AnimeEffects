#ifndef CTRL_POSE_TARGET
#define CTRL_POSE_TARGET

#include "core/ObjectNode.h"

namespace ctrl {
namespace pose {

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

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_TARGET

