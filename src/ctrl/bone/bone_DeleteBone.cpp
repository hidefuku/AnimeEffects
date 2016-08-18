#include "ctrl/bone/bone_DeleteBone.h"

namespace ctrl {
namespace bone {

//-------------------------------------------------------------------------------------------------
DeleteBoneImpl::BoneParent::BoneParent()
    : mTopBones()
    , mBoneTree()
    , mIsTopBone()
{
}

DeleteBoneImpl::BoneParent::BoneParent(QList<core::Bone2*>* aTopBones)
    : mTopBones(aTopBones)
    , mBoneTree()
    , mIsTopBone(true)
{
}

DeleteBoneImpl::BoneParent::BoneParent(core::Bone2::Children* aBoneTree)
    : mTopBones()
    , mBoneTree(aBoneTree)
    , mIsTopBone(false)
{
}

void DeleteBoneImpl::BoneParent::insert(int aIndex, core::Bone2& aBone)
{
    if (mIsTopBone)
    {
        XC_PTR_ASSERT(mTopBones);
        mTopBones->insert(aIndex, &aBone);
    }
    else
    {
        XC_PTR_ASSERT(mBoneTree);
        mBoneTree->insert(aIndex, &aBone);
    }
}

core::Bone2* DeleteBoneImpl::BoneParent::removeAt(int aIndex)
{
    if (mIsTopBone)
    {
        XC_PTR_ASSERT(mTopBones);
        auto bone = mTopBones->at(aIndex);
        XC_PTR_ASSERT(bone);
        mTopBones->removeOne(bone);
        return bone;
    }
    else
    {
        XC_PTR_ASSERT(mBoneTree);
        auto boneIt = mBoneTree->at(aIndex);
        core::Bone2* bone = *boneIt;
        XC_PTR_ASSERT(bone);
        mBoneTree->erase(boneIt);
        return bone;
    }
}

//-------------------------------------------------------------------------------------------------
DeleteBoneImpl::DeleteBoneImpl(QList<core::Bone2*>& aTopBones, core::Bone2& aBone, bool aIsOrigin)
    : mTopBones(aTopBones)
    , mBone(aBone)
    , mParent()
    , mParentIndex()
    , mChildrenCount()
    , mChildrenPos()
    , mGrandChildrenPos()
    , mIsOrigin(aIsOrigin)
    , mDone()
{
}

DeleteBoneImpl::~DeleteBoneImpl()
{
    if (mDone)
    {
        delete &mBone;
    }
}

void DeleteBoneImpl::exec()
{
    if (mBone.parent())
    {
        auto& plist = mBone.parent()->children();
        mParent = BoneParent(&plist);
        mParentIndex = plist.indexOf(&mBone);
    }
    else
    {
        mParent = BoneParent(&mTopBones);
        mParentIndex = mTopBones.indexOf(&mBone);
    }

    mChildrenCount = (int)mBone.children().size();
    for (auto child : mBone.children())
    {
        mChildrenPos.push_back(child->worldPos());
        for (auto grandChild : child->children())
        {
            mGrandChildrenPos.push_back(grandChild->worldPos());
        }
    }

    XC_ASSERT(mParent);
    redo();
}

void DeleteBoneImpl::undo()
{
    // remove target's children from parent
    int grandIndex = 0;
    for (int i = 0; i < mChildrenCount; ++i)
    {
        auto child = mParent.removeAt(mParentIndex);
        XC_PTR_ASSERT(child);

        mBone.children().pushBack(child);
        if (mIsOrigin)
        {
            child->setWorldPos(mChildrenPos.at(i), child->parent());
            child->updateWorldTransform();

            for (auto grandChild : child->children())
            {
                grandChild->setWorldPos(mGrandChildrenPos.at(grandIndex), child);
                grandChild->updateWorldTransform();
                ++grandIndex;
            }
        }
        else
        {
            child->updateWorldTransform();
        }
    }

    // insert target to parent
    mParent.insert(mParentIndex, mBone);
    mBone.updateWorldTransform();

    mDone = false;
}

void DeleteBoneImpl::redo()
{
    // remove target
    {
        auto bone = mParent.removeAt(mParentIndex);
        XC_ASSERT(bone && bone == &mBone);
    }

    // insert target's children to parent
    int index = 0;
    int grandIndex = 0;

    while (!mBone.children().empty())
    {
        core::Bone2* child = mBone.children().popFront();

        mParent.insert(mParentIndex + index, *child);
        if (mIsOrigin)
        {
            child->setWorldPos(mChildrenPos.at(index), child->parent());
            child->updateWorldTransform();

            for (auto grandChild : child->children())
            {
                grandChild->setWorldPos(mGrandChildrenPos.at(grandIndex), child);
                grandChild->updateWorldTransform();
                ++grandIndex;
            }
        }
        else
        {
            child->updateWorldTransform();
        }
        ++index;
    }
    mDone = true;
}

} // namespace bone
} // namespace ctrl
