#ifndef CTRL_MESH_KEYOWNER_H
#define CTRL_MESH_KEYOWNER_H

#include "cmnd/Stack.h"
#include "core/TimeLine.h"
#include "core/MeshKey.h"

namespace ctrl {
namespace mesh {

struct KeyOwner
{
    KeyOwner() : key(), ownsKey() {}

    explicit operator bool() const { return key; }
    bool owns() const { return ownsKey; }
    void pushOwnsKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);
    void deleteOwnsKey();

    core::MeshKey* key;
    bool ownsKey;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_KEYOWNER_H
