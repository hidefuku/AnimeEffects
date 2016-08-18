#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/pose/pose_TransBoneMode.h"
#include "ctrl/bone/bone_Renderer.h"

using namespace core;

namespace ctrl {
namespace pose {

TransBoneMode::TransBoneMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mKeyOwner(aKey)
    , mFocuser()
    , mCommandRef()
    , mMoveOffset()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setFocusConnector(true);
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool TransBoneMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.isLeftPressState())
    {
        mCommandRef = nullptr;
        mFocuser.clearSelection();
        if (focus && focus->parent())
        {
            mFocuser.select(*focus);
            const QVector2D center = focus->parent()->worldPos();
            const QVector2D tail = focus->worldPos();
            const util::Segment2D seg(center, tail - center);

            const QVector2D cursorPos =
                    (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();
            mMoveOffset =
                    cursorPos - util::CollDetect::getPosOnLine(seg, cursorPos);
        }
        updated = true;
    }
    else if (aCursor.isLeftMoveState())
    {
        Bone2* selected = mFocuser.selectingBone();

        if (selected && selected->parent())
        {
            const QVector2D cursorPos =
                    (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();

            const QVector2D center = selected->parent()->worldPos();
            const QVector2D prev = selected->worldPos() - center;
            const QVector2D next = cursorPos - center - mMoveOffset;

            if (std::min(prev.length(), next.length()) >= Constant::dividable())
            {
                const float rotate = util::MathUtil::getAngleDifferenceRad(prev, next);
                rotateBone(*selected, rotate);
            }
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

void TransBoneMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setFocusConnector(true);
    renderer.setTargetMatrix(mTargetMtx);

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderBones(bone);
    }
}

void TransBoneMode::rotateBone(Bone2& aTarget, float aRotate)
{
    cmnd::Stack& stack = mProject.commandStack();
    TimeLine& timeLine = *mTarget.timeLine();
    const int frame = mProject.animator().currentFrame().get();

    const float nextRot = aTarget.rotate() + aRotate;

    // modify
    if (mCommandRef && stack.isModifiable(mCommandRef))
    {
        mCommandRef->modifyValue(nextRot);
        aTarget.updateWorldTransform();

        // notify
        TimeLineEvent event;
        event.setType(TimeLineEvent::Type_ChangeKeyValue);
        event.pushTarget(mTarget, TimeKeyType_Pose, frame);
        mProject.onTimeLineModified(event, false);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, "rotate bone");

        // set notifier
        {
            auto notifier = new TimeLineUtil::Notifier(mProject);
            notifier->event().setType(
                        mKeyOwner.owns() ?
                            TimeLineEvent::Type_PushKey :
                            TimeLineEvent::Type_ChangeKeyValue);
            notifier->event().pushTarget(mTarget, TimeKeyType_Pose, frame);
            macro.grabListener(notifier);
        }
        // push key command
        if (mKeyOwner.owns())
        {
            mKeyOwner.pushOwnsKey(stack, timeLine, frame);
        }

        // push command
        mCommandRef = new RotateBone(&aTarget, nextRot);
        stack.push(mCommandRef);
    }

}

} // namespace pose
} // namespace ctrl
