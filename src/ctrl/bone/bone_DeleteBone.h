#ifndef CTRL_BONE_DELETEBONE
#define CTRL_BONE_DELETEBONE

#include <QVector>
#include "util/TreeUtil.h"
#include "cmnd/Stable.h"
#include "core/BoneKey.h"
#include "core/PoseKey.h"
#include "core/TimeKeyType.h"

namespace ctrl {
namespace bone {

class DeleteBoneImpl : public cmnd::Stable
{
    class BoneParent
    {
        QList<core::Bone2*>* mTopBones;
        core::Bone2::Children* mBoneTree;
        bool mIsTopBone;
    public:
        BoneParent();
        explicit BoneParent(QList<core::Bone2*>* aTopBones);
        explicit BoneParent(core::Bone2::Children* aBoneTree);
        explicit operator bool() const
            { return mIsTopBone ? (bool)mTopBones : (bool)mBoneTree; }
        void insert(int aIndex, core::Bone2& aBone);
        core::Bone2* removeAt(int aIndex);
    };

    QList<core::Bone2*>& mTopBones;
    core::Bone2& mBone;
    BoneParent mParent;
    int mParentIndex;
    int mChildrenCount;
    QVector<QVector2D> mChildrenPos;
    QVector<QVector2D> mGrandChildrenPos;
    bool mIsOrigin;
    bool mDone;

public:
    DeleteBoneImpl(QList<core::Bone2*>& aTopBones, core::Bone2& aBone, bool aIsOrigin);
    virtual ~DeleteBoneImpl();

    virtual void exec();
    virtual void undo();
    virtual void redo();
};

class DeleteBone : public DeleteBoneImpl
{
public:
    DeleteBone(core::BoneKey& aKey, core::Bone2& aBone)
        : DeleteBoneImpl(aKey.data().topBones(), aBone, true)
    {
    }
};

class DeletePose : public DeleteBoneImpl
{
public:
    DeletePose(core::PoseKey& aKey, core::Bone2& aPose)
        : DeleteBoneImpl(aKey.data().topBones(), aPose, false)
    {
    }
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_DELETEBONE

