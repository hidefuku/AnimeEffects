#ifndef CTRL_MESH_TARGET
#define CTRL_MESH_TARGET

#include "core/ObjectNode.h"

namespace ctrl {
namespace mesh {

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

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_TARGET
