#ifndef CTRL_POSE_ROTATEBONE
#define CTRL_POSE_ROTATEBONE

#include "cmnd/Stable.h"
#include "core/Bone2.h"

namespace ctrl {
namespace pose {

class RotateBone : public cmnd::Stable
{
    core::Bone2* mTarget;
    float mPrev;
    float mNext;
    bool mDone;

public:
    RotateBone(core::Bone2* aTarget, float aNext)
        : mTarget(aTarget), mPrev(), mNext(aNext), mDone(false) {}

    void modifyValue(float aNext)
    {
        mNext = aNext;
        if (mDone) redo();
    }

    virtual void exec()
    {
        mPrev = mTarget->rotate();
        redo();
    }

    virtual void undo()
    {
        mTarget->setRotate(mPrev);
        mTarget->updateWorldTransform();
        mDone = false;
    }

    virtual void redo()
    {
        mTarget->setRotate(mNext);
        mTarget->updateWorldTransform();
        mDone = true;
    }
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_ROTATEBONE

