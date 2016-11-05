#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_InfluenceMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

InfluenceMode::InfluenceMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mFocusHAxis()
    , mFocusVAxis()
    , mFocusChild()
    , mCommandRef()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setFocusConnector(true);
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool InfluenceMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.emitsLeftPressedEvent())
    {
        mCommandRef = nullptr;
        mFocuser.clearSelection();
        if (focus && focus->parent())
        {
            const QVector2D curPos =
                    (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();
            const QVector2D pos = focus->worldPos();
            const QVector2D dir = pos - focus->parent()->worldPos();
            const util::Segment2D seg(pos, dir);
            const float length = dir.length();

            if (length >= core::Constant::normalizable())
            {
                mFocuser.select(*focus);

                const bool isLeft = util::CollDetect::getPosSide(seg, curPos) != 1;
                const QVector2D v = dir.normalized();
                const QVector2D h = util::MathUtil::getRotateVector90Deg(v);
                mFocusChild = mFocuser.focusRate() < 0.6f;
                mFocusHAxis = isLeft ? -h : h;
                mFocusVAxis = mFocusChild ? v : -v;
            }
        }
        updated = true;
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {        
        Bone2* selected = mFocuser.selectingBone();
        if (selected && selected->parent())
        {
            const QVector2D vel =
                    (mTargetInvMtx * QVector3D(aCursor.worldVel()) -
                     mTargetInvMtx * QVector3D()).toVector2D();
            const float appendH = QVector2D::dotProduct(mFocusHAxis, vel);
            const float appendV = QVector2D::dotProduct(mFocusVAxis, vel);
            const int index = mFocusChild ? 1 : 0;

            QVector2D newRange = selected->range(index);

            if (std::abs(appendV) > std::abs(appendH))
            {
                newRange.setY(xc_clamp(newRange.y() + appendV, 0.0f, newRange.x()));
            }
            else
            {
                const float vscale = xc_divide(
                            newRange.y(), newRange.x(),
                            Constant::dividable(), 1.0f);

                newRange.setX(std::max(0.0f, newRange.x() + appendH));
                newRange.setY(xc_clamp(vscale, 0.0f, 1.0f) * newRange.x());
            }
            assignInfluence(*selected, index, newRange);
        }
        updated = true;
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        mCommandRef = nullptr;
        mFocuser.clearSelection();
        updated = true;
    }

    return updated;
}

void InfluenceMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setFocusConnector(true);
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

void InfluenceMode::assignInfluence(core::Bone2& aTarget, int aIndex, const QVector2D& aRange)
{
    XC_ASSERT(!mKeyOwner.owns());
    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;

    // modify
    if (mCommandRef && stack.isModifiable(mCommandRef))
    {
        XC_ASSERT(aIndex == mCommandRef->index());
        mCommandRef->modifyValue(aRange);

        // singleshot notify
        Notifier notifier(mProject, mTarget, *mKeyOwner.key, eventType);
        notifier.notify();
    }
    else
    {
        cmnd::ScopedMacro macro(stack, "assign influence");
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

        // push command
        mCommandRef = new AssignInfluence(&aTarget, aIndex, aRange);
        stack.push(mCommandRef);
    }

}

} // namespace bone
} // namespace ctrl
