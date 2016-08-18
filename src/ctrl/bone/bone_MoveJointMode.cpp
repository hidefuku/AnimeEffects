#include "cmnd/ScopedMacro.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_MoveJointMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

MoveJointMode::MoveJointMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mCommandRef()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool MoveJointMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.isLeftPressState())
    {
        mCommandRef = nullptr;
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        if (focus)
        {
            mFocuser.select(*focus);
        }
        updated = true;
    }
    else if (aCursor.isLeftMoveState())
    {
        Bone2* selected = mFocuser.selectingBone();
        if (selected)
        {
            moveBone(*selected, aCursor.worldVel());
        }
        updated = true;
    }
    else if (aCursor.isLeftReleaseState())
    {
        mCommandRef = nullptr;
        mFocuser.clearSelection();
        updated = true;
    }

    return updated;
}

void MoveJointMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderInfluence(bone);
    }

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderBones(bone);
    }
}

void MoveJointMode::moveBone(core::Bone2& aTarget, const QVector2D& aMove)
{
    XC_ASSERT(!mKeyOwner.owns());
    cmnd::Stack& stack = mProject.commandStack();
    const QVector2D move =
            (mTargetInvMtx * QVector3D(aMove) -
             mTargetInvMtx * QVector3D()).toVector2D();
    const QVector2D nextPos = aTarget.worldPos() + move;
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;

    // modify
    if (mCommandRef && stack.isModifiable(mCommandRef))
    {
        mCommandRef->modifyValue(nextPos);

        // singleshot notify
        Notifier notifier(mProject, mTarget, *mKeyOwner.key, eventType);
        notifier.notify();
    }
    else
    {
        cmnd::ScopedMacro macro(stack, "move bone");
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

        // push command
        mCommandRef = new MoveBone(&aTarget, nextPos, true);
        stack.push(mCommandRef);
    }
}

} // namespace bone
} // namespace ctrl
