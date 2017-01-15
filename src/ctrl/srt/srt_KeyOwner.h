#ifndef CTRL_SRT_KEYOWNER_H
#define CTRL_SRT_KEYOWNER_H

#include "cmnd/Stack.h"
#include "core/TimeLine.h"
#include "core/MoveKey.h"
#include "core/RotateKey.h"
#include "core/ScaleKey.h"
namespace core { class TimeKeyExpans; }

namespace ctrl {
namespace srt {

struct KeyOwner
{
    KeyOwner();
    ~KeyOwner();

    explicit operator bool() const { return moveKey && rotateKey && scaleKey; }
    bool ownsSomeKeys() const { return ownsMoveKey || ownsRotateKey || ownsScaleKey; }
    void deleteOwningKeys();

    void pushOwningMoveKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);
    void pushOwningRotateKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);
    void pushOwningScaleKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame);

    bool updatePosture(const core::TimeKeyExpans& aExpans);

    QMatrix4x4 getLocalSRTMatrixFromKeys() const;
    QMatrix4x4 getLocalSRMatrixFromKeys() const;

    core::MoveKey* moveKey;
    core::RotateKey* rotateKey;
    core::ScaleKey* scaleKey;
    bool ownsMoveKey;
    bool ownsRotateKey;
    bool ownsScaleKey;

    QMatrix4x4 parentMtx;
    QMatrix4x4 invParentMtx;
    QMatrix4x4 invParentSRMtx;
    QMatrix4x4 locSRMtx;
    QMatrix4x4 locSRTMtx;
    QMatrix4x4 locCSRTMtx;
    bool hasInv;
};

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_KEYOWNER_H
