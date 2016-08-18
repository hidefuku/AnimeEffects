#ifndef CTRL_POSE_KEYOWNER_H
#define CTRL_POSE_KEYOWNER_H

#include "cmnd/Stack.h"
#include "core/TimeLine.h"
#include "core/PoseKey.h"

namespace ctrl {
namespace pose {

struct KeyOwner
{
    KeyOwner() : key(), ownsKey(), parent() {}

    explicit operator bool() const { return key; }
    bool owns() const { return ownsKey; }
    void pushOwnsKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);
    void deleteOwnsKey();

    core::PoseKey* key;
    bool ownsKey;
    core::TimeKey* parent;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_KEYOWNER_H
