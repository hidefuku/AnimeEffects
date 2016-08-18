#ifndef CTRL_BONE_KEYOWNER_H
#define CTRL_BONE_KEYOWNER_H

#include "cmnd/Stack.h"
#include "core/TimeLine.h"
#include "core/BoneKey.h"

namespace ctrl {
namespace bone {

struct KeyOwner
{
    KeyOwner() : key(), ownsKey() {}

    explicit operator bool() const { return key; }
    bool owns() const { return ownsKey; }
    void pushOwnsKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);
    void deleteOwnsKey();

    core::BoneKey* key;
    bool ownsKey;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_KEYOWNER_H

