#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "core/ObjectNodeUtil.h"
#include "core/TimeKeyExpans.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/srt/srt_CentroidMode.h"

namespace
{
static const float kTransRange = 3.0f;
static const float kCrossRadius = 30.0f;
static const float kCrossSub = 8.0f;
}

using namespace core;

namespace ctrl {
namespace srt {

CentroidMode::CentroidMode(Project& aProject, ObjectNode& aTarget, KeyOwner& aKeyOwner)
    : mProject(aProject)
    , mTarget(aTarget)
    , mKeyOwner(aKeyOwner)
    , mFocusing()
    , mMoving()
    , mBaseVec()
    , mBasePosition()
    , mBaseCentroid()
    , mCommandRef()
    , mAdjustPosition()
{
    XC_PTR_ASSERT(mTarget.timeLine());
}

void CentroidMode::updateParam(const SRTParam& aParam)
{
    mAdjustPosition = aParam.adjustPosition;
}

bool CentroidMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto parentMtx = mKeyOwner.mtx;
    auto worldSRMtx = mKeyOwner.mtx * mKeyOwner.locSRMtx;
    bool hasParentInv = false;
    bool hasWorldInv = false;
    auto parentInvMtx = parentMtx.inverted(&hasParentInv);
    auto worldSRInvMtx = worldSRMtx.inverted(&hasWorldInv);

    auto curPos = aCursor.worldPos();
    auto center = getWorldCentroidPos();
    const bool prevFocus = mFocusing;
    mFocusing = aCamera.toScreenLength((center - curPos).length()) <= kCrossRadius;
    bool mod = (prevFocus != mFocusing);

    if (aCursor.emitsLeftPressedEvent())
    {
        if (mFocusing && hasParentInv && hasWorldInv)
        {        
            mMoving = true;
            mBaseVec = center - curPos;
            mBaseCentroid = (worldSRInvMtx * QVector3D(center)).toVector2D();
            mBasePosition = (parentInvMtx * QVector3D(center)).toVector2D();
            mCommandRef = nullptr;
        }
        mod = true;
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        if (mMoving && hasParentInv && hasWorldInv)
        {
            auto newCentroid = (worldSRInvMtx * QVector3D(curPos + mBaseVec)).toVector2D();
            auto newPosition = (parentInvMtx * QVector3D(curPos + mBaseVec)).toVector2D();
            moveCentroid(newCentroid, newPosition);
            mKeyOwner.updatePosture(mTarget.timeLine()->current());
        }
        mod = true;
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        mCommandRef = nullptr;
        mMoving = false;
        mod = true;
    }

    return mod;
}

void CentroidMode::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    const QColor idleColor(100, 100, 255, 255);
    const QColor focusColor(255, 255, 255, 255);
    const QBrush brush((mFocusing || mMoving) ? focusColor : idleColor);
    const QPointF c = aInfo.camera.toScreenPos(getWorldCentroidPos().toPointF());
    const QPointF h(kCrossRadius, 0.0f);
    const QPointF v(0.0f, kCrossRadius);
    const QPointF hs(kCrossSub, 0.0f);
    const QPointF vs(0.0f, kCrossSub);

    aPainter.setPen(QPen(brush, 1.5f, Qt::SolidLine));
    aPainter.setBrush(brush);
    aPainter.drawEllipse(c, kTransRange, kTransRange);

    aPainter.setPen(QPen(brush, 1.5f, Qt::DashLine));
    aPainter.drawLine(c - h, c - hs);
    aPainter.drawLine(c + h, c + hs);
    aPainter.drawLine(c - v, c - vs);
    aPainter.drawLine(c + v, c + vs);
}

void CentroidMode::moveCentroid(const QVector2D& aNewCentroid, const QVector2D& aNewPosition)
{
    XC_PTR_ASSERT(mTarget.timeLine());

    cmnd::Stack& stack = mProject.commandStack();
    const int frame = mProject.animator().currentFrame().get();
    auto centroidMove = aNewCentroid - mBaseCentroid;
    auto positionMove = aNewPosition - mBasePosition;

    if (mCommandRef && mProject.commandStack().isModifiable(mCommandRef))
    {
        // modify command
        mCommandRef->modifyValue(centroidMove, positionMove);

        // singleshot notify
        TimeLineEvent event;
        event.setType(TimeLineEvent::Type_ChangeKeyValue);
        event.pushTarget(mTarget, TimeKeyType_Move, frame);
        mProject.onTimeLineModified(event, false);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("update centroid of a moving key"));

        // set notifier
        {
            auto notifier = new TimeLineUtil::Notifier(mProject);
            notifier->event().setType(
                        mKeyOwner.ownsMoveKey ?
                            TimeLineEvent::Type_PushKey :
                            TimeLineEvent::Type_ChangeKeyValue);
            notifier->event().pushTarget(mTarget, TimeKeyType_Move, frame);
            macro.grabListener(notifier);
        }
        // push owning key if necessary
        mKeyOwner.pushOwningMoveKey(stack, *mTarget.timeLine(), frame);

        // create move command
        mCommandRef = new CentroidMover(
                    mProject, mTarget, centroidMove,
                    positionMove, frame, mAdjustPosition);
        stack.push(mCommandRef);
    }
}

QVector2D CentroidMode::getWorldCentroidPos() const
{
    return (mKeyOwner.mtx * mKeyOwner.locMtx * QVector3D()).toVector2D();
}

} // namespace srt
} // namespace ctrl
