#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/bone/bone_PaintInflMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

PaintInflMode::PaintInflMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mCommandRef()
    , mBrush()
    , mBrushPressure()
    , mIsBrushDrawing()
{
    XC_PTR_ASSERT(mKeyOwner.key);
}

void PaintInflMode::updateParam(const BoneParam& aParam)
{
    mBrush.setRadius(aParam.piRadius);
    mBrushPressure = aParam.piPressure;
}

bool PaintInflMode::updateCursor(const CameraInfo&, const AbstractCursor& aCursor)
{
    mBrush.setCenter(aCursor.worldPos());

    if (aCursor.emitsLeftPressedEvent())
    {
        mIsBrushDrawing = true;
        mCommandRef = nullptr;
        updatePaint();
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        updatePaint();
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        mIsBrushDrawing = false;
        mCommandRef = nullptr;
        updatePaint();
    }

    return true;
}

bool PaintInflMode::updatePaint()
{
    bool modified = false;
    const QVector2D brushPos = (mTargetInvMtx * QVector3D(mBrush.center())).toVector2D();

    const float brushRange =
            (mTargetInvMtx * QVector3D(mBrush.radius(), 0.0f, 0.0f) -
             mTargetInvMtx * QVector3D()).toVector2D().length();
    const float brushCoreRange = 0.3f * brushRange;

    auto& topBones = mKeyOwner.key->data().topBones();
    for (auto topBone : topBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            auto parent = bone->parent();
            if (!parent) continue;

            const QVector2D ppos = parent->worldPos();
            const QVector2D cpos = bone->worldPos();
            const util::Segment2D boneSeg(ppos, cpos- ppos);
            const float ratio = util::CollDetect::getRawSegmentRate(boneSeg, brushPos);

            if (!xc_contains(ratio, 0.0f, 1.0f)) continue;

            util::Segment2D v;
            v.start = boneSeg.start + boneSeg.dir * ratio;
            v.dir = brushPos - v.start;
            const float vlen = v.dir.length();

            auto linearRange = bone->blendedRange(ratio);
            const float append = (vlen + brushRange) - linearRange.x();

            if (append < 0) continue;
            if (vlen - brushRange > std::max(linearRange.x(), brushCoreRange)) continue;

            std::array<QVector2D, 2> prevRange = { bone->range(0), bone->range(1) };
            std::array<QVector2D, 2> nextRange = prevRange;
            for (int i = 0; i < 2; ++i)
            {
                const float rangeAdd = mBrushPressure * append * (i == 0 ? (1.0f - ratio) : ratio);
                nextRange[i].setX(std::min(nextRange[i].x() + rangeAdd, brushRange));
                nextRange[i].setY(std::min(nextRange[i].y() + rangeAdd, brushRange));
            }
            assignInfluence(*bone, prevRange, nextRange);
            modified = true;
        }
    }

    if (modified)
    {
        notifyAssign();
    }

    return modified;
}

void PaintInflMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
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

    renderer.renderBrush(mBrush, mIsBrushDrawing);
}

void PaintInflMode::assignInfluence(
        core::Bone2& aTarget, const std::array<QVector2D, 2>& aPrev,
        const std::array<QVector2D, 2>& aNext)
{
    XC_ASSERT(!mKeyOwner.owns());
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;
    cmnd::Stack& stack = mProject.commandStack();

    // modify
    if (mCommandRef && stack.isModifiable(mCommandRef))
    {
        mCommandRef->push(aTarget, aPrev, aNext);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("update influence of a bone"));
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

        // push command
        mCommandRef = new FuzzyAssignInfluence();
        mCommandRef->push(aTarget, aPrev, aNext);
        stack.push(mCommandRef);
    }
}

void PaintInflMode::notifyAssign()
{
    // singleshot notify
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;
    Notifier notifier(mProject, mTarget, *mKeyOwner.key, eventType);
    notifier.notify();
}

} // namespace bone
} // namespace ctrl
