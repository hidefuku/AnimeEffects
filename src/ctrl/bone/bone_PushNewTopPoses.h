#ifndef CTRL_BONE_PUSHNEWTOPPOSES
#define CTRL_BONE_PUSHNEWTOPPOSES

#include <QVector>
#include "util/TreeUtil.h"
#include "cmnd/Stable.h"
#include "core/BoneKey.h"
#include "core/PoseKey.h"
#include "core/TimeKeyType.h"

namespace ctrl {
namespace bone {

class PushNewTopPoses : public cmnd::Stable
{
    core::BoneKey* mTarget;
    core::Bone2* mNewTopBone;
    int mTopIndex;
    QVector<core::Bone2*> mPoses;
    bool mDone;

public:
    PushNewTopPoses(core::BoneKey* aTarget, core::Bone2* aNewTopBone)
        : mTarget(aTarget)
        , mNewTopBone(aNewTopBone)
        , mTopIndex()
        , mPoses()
        , mDone(false)
    {
        XC_PTR_ASSERT(aTarget);
        XC_PTR_ASSERT(aNewTopBone);
    }

    ~PushNewTopPoses()
    {
        if (!mDone)
        {
            qDeleteAll(mPoses);
        }
    }

    virtual void exec()
    {
        mTopIndex = 0;
        for (auto bone : mTarget->data().topBones())
        {
            if (bone == mNewTopBone) break;
            ++mTopIndex;
        }
        XC_ASSERT(mTopIndex < mTarget->data().topBones().size());

        for (auto child : mTarget->children())
        {
            if (child->type() == core::TimeKeyType_Pose)
            {
                mPoses.push_back(mNewTopBone->createShadow());
            }
        }

        redo();
    }

    virtual void undo()
    {
        int index = 0;
        for (auto child : mTarget->children())
        {
            if (child->type() == core::TimeKeyType_Pose)
            {
                auto posekey = (core::PoseKey*)child;
                posekey->data().topBones().removeAt(mTopIndex);
                ++index;
            }
        }
        mDone = false;
    }

    virtual void redo()
    {
        int index = 0;
        for (auto child : mTarget->children())
        {
            if (child->type() == core::TimeKeyType_Pose)
            {
                auto posekey = (core::PoseKey*)child;
                posekey->data().topBones().insert(mTopIndex, mPoses.at(index));
                mPoses.at(index)->updateWorldTransform();
                ++index;
            }
        }
        mDone = true;
    }
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_PUSHNEWTOPPOSES

