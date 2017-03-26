#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "util/TreeUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/pose/pose_DrawBoneMode.h"
#include "ctrl/pose/pose_BoneDynamics.h"
#include "ctrl/bone/bone_Renderer.h"

using namespace core;

namespace ctrl {
namespace pose {

DrawBoneMode::DrawBoneMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mKeyOwner(aKey)
    , mFocuser()
    , mCommandRef()
    , mPullPos()
    , mPullOffset()
    , mPullPosRate()
    , mPullWeight(0.0f)
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setFocusConnector(true);
    mFocuser.setTargetMatrix(mTargetMtx);
}

void DrawBoneMode::updateParam(const PoseParam& aParam)
{
    mPullWeight = aParam.diWeight;
}

bool DrawBoneMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.emitsLeftPressedEvent())
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
            mPullPos = util::CollDetect::getPosOnLine(seg, cursorPos);
            mPullOffset = cursorPos - mPullPos;
            mPullPosRate = (mPullPos - seg.start).length() / seg.dir.length();
        }
        updated = true;
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        Bone2* selected = mFocuser.selectingBone();

        if (selected && selected->parent())
        {
            const QVector2D cursorPos =
                    (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();

            auto nextPos = (cursorPos - mPullOffset);
            auto pull = nextPos - mPullPos;
            mPullPos = nextPos;
            pullBone(*selected, pull, mPullPosRate);
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

void DrawBoneMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
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

void DrawBoneMode::pullBone(Bone2& aTarget, const QVector2D& aPull, float aPullPosRate)
{
    auto& targetRoot = util::TreeUtil::getTreeRoot<Bone2>(aTarget);
    BoneDynamics dynamics(targetRoot);
    dynamics.setConduction(1.0f - mPullWeight);
    {
        RigidBone* rigidTarget = nullptr;
        RigidBone::Iterator itr(&dynamics.rigidTopBone());
        while (itr.hasNext())
        {
            auto rigidBone = itr.next();
            if (rigidBone->ptr == &aTarget)
            {
                rigidTarget = rigidBone;
                break;
            }
        }
        if (rigidTarget)
        {
            for (int i = 0; i < 16; ++i)
            {
                dynamics.pullBone(*rigidTarget, aPull / 16.0f, aPullPosRate);
            }
        }
    }

    // get next rotation values
    QVector<float> nextRots;
    {
        auto diffs = dynamics.rotationDifferences();
        Bone2::ConstIterator itr(&targetRoot);

        for (auto diff : diffs)
        {
            if (!itr.hasNext()) break;
            nextRots.push_back(itr.next()->rotate() + diff);
        }
    }

    // create a command
    {
        cmnd::Stack& stack = mProject.commandStack();
        TimeLine& timeLine = *mTarget.timeLine();
        const int frame = mProject.animator().currentFrame().get();

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
            cmnd::ScopedMacro macro(stack, CmndName::tr("pull bones of a posing key"));

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
            mCommandRef = new RotateBones(&targetRoot, nextRots);
            stack.push(mCommandRef);
        }
    }
}

} // namespace pose
} // namespace ctrl
