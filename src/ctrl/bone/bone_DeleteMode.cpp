#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/TimeLine.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/bone/bone_DeleteMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_PushNewPoses.h"
#include "ctrl/bone/bone_PushNewTopPoses.h"
#include "ctrl/bone/bone_Notifier.h"
#include "ctrl/bone/bone_DeleteBone.h"

using namespace core;

namespace ctrl {
namespace bone {

DeleteMode::DeleteMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool DeleteMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.emitsLeftPressedEvent())
    {
        if (focus)
        {
            mFocuser.clearFocus();
            deleteBone(*focus);
            updated = true;
        }
    }

    return updated;
}

void DeleteMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
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

void DeleteMode::deleteBone(Bone2& aBone)
{
    XC_ASSERT(!mKeyOwner.owns());
    cmnd::Stack& stack = mProject.commandStack();
    BoneKey& key = *mKeyOwner.key;

    cmnd::ScopedMacro macro(stack, CmndName::tr("delete a bone"));

    // set notifier
    macro.grabListener(new Notifier(mProject, mTarget, key, TimeLineEvent::Type_ChangeKeyValue));

    // push command
    {
        auto treePos = util::TreeUtil::getTreePos<Bone2>(&aBone);
        Bone2& treeTop = util::TreeUtil::getTreeRoot(aBone);
        const int topIndex = key.data().topBones().indexOf(&treeTop);

        stack.push(new DeleteBone(key, aBone));

        for (auto keyChild : key.children())
        {
            if (keyChild->type() == core::TimeKeyType_Pose)
            {
                PoseKey& poseKey = *static_cast<PoseKey*>(keyChild);
                Bone2* poseTop = poseKey.data().topBones().at(topIndex);
                XC_PTR_ASSERT(poseTop);
                Bone2* pose = util::TreeUtil::find(*poseTop, treePos);
                XC_PTR_ASSERT(pose);
                stack.push(new DeletePose(poseKey, *pose));
            }
        }
    }
}

} // namespace bone
} // namespace ctrl

