#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_EraseInflMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

EraseInflMode::EraseInflMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
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

void EraseInflMode::updateParam(const BoneParam& aParam)
{
    mBrush.setRadius(aParam.eiRadius);
    mBrushPressure = aParam.eiPressure;
}

bool EraseInflMode::updateCursor(const CameraInfo&, const AbstractCursor& aCursor)
{
    mBrush.setCenter(aCursor.worldPos());

    if (aCursor.isLeftPressState())
    {
        mIsBrushDrawing = true;
        mCommandRef = nullptr;
        updatePaint();
    }
    else if (aCursor.isLeftMoveState())
    {
        updatePaint();
    }
    else if (aCursor.isLeftReleaseState())
    {
        mIsBrushDrawing = false;
        mCommandRef = nullptr;
        updatePaint();
    }

    return true;
}

bool EraseInflMode::updatePaint()
{
    bool modified = false;

    auto& topBones = mKeyOwner.key->data().topBones();
    for (auto topBone : topBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();

            auto sinkRes = getSink(*bone, mBrush);
            auto sink = sinkRes.first;
            auto ratio = sinkRes.second;

            if (sink <= 0.0f) continue;

            std::array<QVector2D, 2> prevRange = { bone->range(0), bone->range(1) };
            std::array<QVector2D, 2> nextRange = prevRange;
            for (int i = 0; i < 2; ++i)
            {
                auto sub = mBrushPressure * sink * (i == 0 ? (1.0f - ratio) : ratio);
                nextRange[i].setX(std::max(nextRange[i].x() - sub, 0.0f));
                if (nextRange[i].y() > nextRange[i].x())
                {
                    nextRange[i].setY(nextRange[i].x());
                }
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

void EraseInflMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
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

std::pair<float, float> EraseInflMode::getSink(
        const core::Bone2& aBone, const util::Circle& aBrush) const
{
    std::pair<float, float> result(0.0f, 0.0f);

    auto parent = aBone.parent();
    if (!parent) return result;

    const QVector2D brushPos = (mTargetInvMtx * QVector3D(aBrush.center())).toVector2D();
    const float brushRange =
            (mTargetInvMtx * QVector3D(mBrush.radius(), 0.0f, 0.0f) -
             mTargetInvMtx * QVector3D()).toVector2D().length();

    const QVector2D ppos = parent->worldPos();
    const QVector2D cpos = aBone.worldPos();
    const util::Segment2D boneSeg(ppos, cpos- ppos);

    const float ratio = util::CollDetect::getRawSegmentRate(boneSeg, brushPos);
    auto linearRange = aBone.blendedRange(ratio);


    if (ratio < 0.0f)
    {
        auto len = (brushPos - ppos).length() - brushRange;
        if (len < linearRange.x())
        {
            result.first = linearRange.x() - len;
            result.second = 0.0f;
        }
    }
    else if (ratio < 1.0f)
    {
        auto v = brushPos - (boneSeg.start + boneSeg.dir * ratio);
        auto len = v.length() - brushRange;

        if (len < linearRange.x())
        {
            result.first = linearRange.x() - len;
            result.second = ratio;
        }
    }
    else
    {
        auto len = (brushPos - cpos).length() - brushRange;
        if (len < linearRange.x())
        {
            result.first = linearRange.x() - len;
            result.second = 1.0f;
        }
    }

    return result;
}

void EraseInflMode::assignInfluence(
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
        cmnd::ScopedMacro macro(stack, "assign influence");
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

        // push command
        mCommandRef = new FuzzyAssignInfluence();
        mCommandRef->push(aTarget, aPrev, aNext);
        stack.push(mCommandRef);
    }
}

void EraseInflMode::notifyAssign()
{
    // singleshot notify
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;
    Notifier notifier(mProject, mTarget, *mKeyOwner.key, eventType);
    notifier.notify();
}

} // namespace bone
} // namespace ctrl
