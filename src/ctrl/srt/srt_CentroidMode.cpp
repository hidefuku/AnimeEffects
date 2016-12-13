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
    , mCommandRef()
    , mAdjustPostures()
{
    XC_PTR_ASSERT(mTarget.timeLine());
}

void CentroidMode::updateParam(const SRTParam& aParam)
{
    mAdjustPostures = aParam.adjustExistingPostures;
}

bool CentroidMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto worldMtx = mKeyOwner.mtx * mKeyOwner.locSRMtx;
    bool hasWorldInv = false;
    auto worldInvMtx = worldMtx.inverted(&hasWorldInv);

    auto curPos = aCursor.worldPos();
    auto center = getWorldCentroidPos();
    const bool prevFocus = mFocusing;
    mFocusing = aCamera.toScreenLength((center - curPos).length()) <= kCrossRadius;
    bool mod = (prevFocus != mFocusing);

    if (aCursor.emitsLeftPressedEvent())
    {
        if (mFocusing && hasWorldInv)
        {        
            mMoving = true;
            mBaseVec = center - curPos;
            mBaseCenter = (worldInvMtx * QVector3D(center)).toVector2D();
            mCommandRef = nullptr;
        }
        mod = true;
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        if (mMoving && hasWorldInv)
        {
            auto newLocalCenter = worldInvMtx * QVector3D(curPos + mBaseVec);
            moveCentroid(newLocalCenter.toVector2D());
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

void CentroidMode::moveCentroid(const QVector2D& aNewCenter)
{
    XC_PTR_ASSERT(mTarget.timeLine());

    const QVector2D newCenter(
                xc_clamp(aNewCenter.x(), Constant::transMin(), Constant::transMax()),
                xc_clamp(aNewCenter.y(), Constant::transMin(), Constant::transMax()));

    cmnd::Stack& stack = mProject.commandStack();

    if (mCommandRef && mProject.commandStack().isModifiable(mCommandRef))
    {
        // modify command
        mCommandRef->modifyValue(newCenter);

        // singleshot notify
        TimeLineEvent event;
        event.setType(TimeLineEvent::Type_ChangeKeyValue);
        CentroidMover::pushEventTargets(mTarget, event, mCommandRef->adjustsPostures());
        mProject.onTimeLineModified(event, false);
        mProject.onNodeAttributeModified(mTarget, false); ///@todo Is there any means?
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("move a centroid"));

        // set notifier
        {
            auto tln = new TimeLineUtil::Notifier(mProject);
            tln->event().setType(TimeLineEvent::Type_ChangeKeyValue);
            CentroidMover::pushEventTargets(mTarget, tln->event(), mAdjustPostures);
            macro.grabListener(tln);
        }
        macro.grabListener(new ObjectNodeUtil::AttributeNotifier(mProject, mTarget));

        // create command
        mCommandRef = new CentroidMover(mProject, mTarget, mBaseCenter,
                                        newCenter, mAdjustPostures);

        // push command
        stack.push(mCommandRef);
    }
}

QVector2D CentroidMode::getWorldCentroidPos() const
{
    return (mKeyOwner.mtx * mKeyOwner.locMtx * QVector3D()).toVector2D();
}

} // namespace srt
} // namespace ctrl
