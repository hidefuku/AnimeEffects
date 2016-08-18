#ifndef CTRL_BONE_PUSHNEWPOSES
#define CTRL_BONE_PUSHNEWPOSES

#include <QVector>
#include "util/TreeUtil.h"
#include "cmnd/Stable.h"
#include "core/BoneKey.h"
#include "core/PoseKey.h"
#include "core/TimeKeyType.h"

namespace ctrl {
namespace bone {

class PushNewPoses : public cmnd::Stable
{
    core::BoneKey* mTarget;
    core::Bone2* mNewBone;
    util::TreePos mPos;
    int mTopIndex;
    QVector<core::Bone2*> mPoses;
    bool mDone;

public:
    PushNewPoses(core::BoneKey* aTarget, core::Bone2* aNewBone)
        : mTarget(aTarget)
        , mNewBone(aNewBone)
        , mPos()
        , mTopIndex()
        , mPoses()
        , mDone(false)
    {
        XC_PTR_ASSERT(aTarget);
        XC_PTR_ASSERT(aNewBone);
    }

    ~PushNewPoses()
    {
        if (!mDone)
        {
            qDeleteAll(mPoses);
        }
    }

    virtual void exec()
    {
        mPos = util::TreeUtil::getTreePos<core::Bone2>(mNewBone);
        XC_ASSERT(mPos.isValid());

        mTopIndex = 0;
        core::Bone2& root = util::TreeUtil::getTreeRoot(*mNewBone);
        for (auto bone : mTarget->data().topBones())
        {
            if (bone == &root) break;
            ++mTopIndex;
        }
        XC_ASSERT(mTopIndex < mTarget->data().topBones().size());

        for (auto child : mTarget->children())
        {
            if (child->type() == core::TimeKeyType_Pose)
            {
                mPoses.push_back(mNewBone->createShadow());
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
                core::Bone2* root = ((core::PoseKey*)child)->data().topBones().at(mTopIndex);
                XC_PTR_ASSERT(root);

                util::TreeUtil::eraseFrom(*root, mPos);
                root->updateWorldTransform();
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
                core::Bone2* root = ((core::PoseKey*)child)->data().topBones().at(mTopIndex);
                XC_PTR_ASSERT(root);

                util::TreeUtil::insertTo(*root, mPos, mPoses.at(index));
                root->updateWorldTransform();
                ++index;
            }
        }
        mDone = true;
    }
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_PUSHNEWPOSES

