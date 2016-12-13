#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/TimeLine.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/bone/bone_CreateMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_PushNewPoses.h"
#include "ctrl/bone/bone_PushNewTopPoses.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

CreateMode::CreateMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mDangledTop()
    , mFocuser()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool CreateMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    // clear selecting
    if (aCursor.emitsRightReleasedEvent())
    {
        mFocuser.clearSelection();
        updated = true;
    }

    if (aCursor.emitsLeftReleasedEvent())
    {
        if (focus)
        {
            // change selection
            mFocuser.clearFocus();
            mFocuser.clearSelection();
            mFocuser.select(*focus);
        }
        else
        {
            const QVector2D pos = (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();

            Bone2* selected = mFocuser.selectingBone();
            mFocuser.clearSelection();

            if (selected)
            {
                // append bone
                Bone2* bone = new Bone2();
                bone->setWorldPos(pos, selected);
                mFocuser.select(*bone);
                pushNewBone(*selected, *bone);
                bone->updateWorldTransform();
            }
            else
            {
                if (!mDangledTop)
                {
                    mDangledTop.reset(new Bone2());
                    mDangledTop->setWorldPos(pos, nullptr);
                    mDangledTop->updateWorldTransform();
                    mFocuser.select(*mDangledTop);
                }
                else
                {
                    // add new top bone
                    Bone2* bone = new Bone2();
                    bone->setWorldPos(pos, mDangledTop.data());
                    mFocuser.select(*bone);
                    pushNewTopBone(*mDangledTop.take(), *bone);
                    bone->updateWorldTransform();
                }
            }
        }
        updated = true;
    }

    if (mDangledTop && !mDangledTop->isSelected())
    {
        mDangledTop.reset();
    }

    return updated;
}

void CreateMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
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

    if (mDangledTop)
    {
        renderer.renderJoint(*mDangledTop);
    }
}


void CreateMode::pushNewBone(core::Bone2& aParent, core::Bone2& aNewChild)
{
    XC_ASSERT(!mKeyOwner.owns());
    //const int frame = mProject.animator().currentFrame();
    //TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();

    cmnd::ScopedMacro macro(stack, CmndName::tr("push new bone"));

    // set notifier
    macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, TimeLineEvent::Type_ChangeKeyValue));

    // push command
    stack.push(new cmnd::PushBackNewTreeObject<Bone2>(&aParent.children(), &aNewChild));
    stack.push(new PushNewPoses(mKeyOwner.key, &aNewChild));
}

void CreateMode::pushNewTopBone(Bone2& aNewRoot, Bone2& aNewChild)
{
    const int frame = mProject.animator().currentFrame().get();
    TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = mKeyOwner.owns() ?
                TimeLineEvent::Type_PushKey :
                TimeLineEvent::Type_ChangeKeyValue;

    cmnd::ScopedMacro macro(stack, CmndName::tr("push new top bone"));

    // set notifier
    macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

    // push key command
    if (mKeyOwner.owns())
    {
        mKeyOwner.pushOwnsKey(stack, timeLine, frame);
    }

    // push command
    stack.push(new cmnd::GrabNewObject<Bone2>(&aNewRoot));
    stack.push(new cmnd::PushBackList<Bone2*>(&mKeyOwner.key->data().topBones(), &aNewRoot));
    stack.push(new cmnd::PushBackNewTreeObject<Bone2>(&aNewRoot.children(), &aNewChild));
    stack.push(new PushNewTopPoses(mKeyOwner.key, &aNewRoot));
}

} // namespace bone
} // namespace ctrl

