#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/pose/pose_ErasePoseMode.h"
#include "ctrl/bone/bone_Renderer.h"

using namespace core;

namespace ctrl {
namespace pose {

ErasePoseMode::ErasePoseMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mKeyOwner(aKey)
    , mCommandRef()
    , mBrush()
    , mBrushPressure()
    , mIsBrushDrawing()
{
    XC_PTR_ASSERT(mKeyOwner.key);
}

void ErasePoseMode::updateParam(const PoseParam& aParam)
{
    mBrush.setRadius(aParam.eiRadius);
    mBrushPressure = aParam.eiPressure;
}

bool ErasePoseMode::updateCursor(const CameraInfo&, const AbstractCursor& aCursor)
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
    }

    return true;
}

void ErasePoseMode::updatePaint()
{
    cmnd::Stack& stack = mProject.commandStack();
    TimeLine& timeLine = *mTarget.timeLine();
    const int frame = mProject.animator().currentFrame().get();
    auto& topBones = mKeyOwner.key->data().topBones();

    QVector<float> nextRots;
    {
        auto eraseRate = 1.0f - mBrushPressure;

        for (auto topBone : topBones)
        {
            Bone2::ConstIterator itr(topBone);
            while (itr.hasNext())
            {
                auto ptr = itr.next();
                auto worldPos = mTargetMtx * QVector3D(ptr->worldPos());
                if (mBrush.isInside(worldPos.toVector2D()))
                {
                    nextRots.push_back(ptr->rotate() * eraseRate);
                }
                else
                {
                    nextRots.push_back(ptr->rotate());
                }
            }
        }
    }

    // modify
    if (mCommandRef && stack.isModifiable(mCommandRef))
    {
        mCommandRef->modifyValue(nextRots);

        // notify
        TimeLineEvent event;
        event.setType(TimeLineEvent::Type_ChangeKeyValue);
        event.pushTarget(mTarget, TimeKeyType_Pose, frame);
        mProject.onTimeLineModified(event, false);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("erase a pose"));

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
        mCommandRef = new RotateAllBones(topBones, nextRots);
        stack.push(mCommandRef);
    }
}

void ErasePoseMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    //renderer.setFocusConnector(true);
    renderer.setTargetMatrix(mTargetMtx);

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderBones(bone);
    }

    renderer.renderBrush(mBrush, mIsBrushDrawing);
}

} // namespace pose
} // namespace ctrl
