#include "ctrl/pose/pose_RotateBones.h"

namespace ctrl {
namespace pose {

//-------------------------------------------------------------------------------------------------
RotateBones::RotateBones(core::Bone2* aRootTarget, const QVector<float>& aNexts)
    : mRootTarget(aRootTarget)
    , mPrevs()
    , mNexts(aNexts)
    , mDone(false)
{
}

void RotateBones::modifyValue(const QVector<float>& aNexts)
{
    mNexts = aNexts;
    if (mDone) redo();
}

void RotateBones::exec()
{
    core::Bone2::Iterator itr(mRootTarget);
    while (itr.hasNext())
    {
        mPrevs.push_back(itr.next()->rotate());
    }
    redo();
}

void RotateBones::undo()
{
    core::Bone2::Iterator itr(mRootTarget);
    for (auto rotate : mPrevs)
    {
        if (!itr.hasNext()) break;
        itr.next()->setRotate(rotate);
    }
    mRootTarget->updateWorldTransform();
    mDone = false;
}

void RotateBones::redo()
{
    core::Bone2::Iterator itr(mRootTarget);
    for (auto rotate : mNexts)
    {
        if (!itr.hasNext()) break;
        itr.next()->setRotate(rotate);
    }
    mRootTarget->updateWorldTransform();
    mDone = true;
}

//-------------------------------------------------------------------------------------------------
RotateAllBones::RotateAllBones(QList<core::Bone2*>& aTopBones, const QVector<float>& aNexts)
    : mTopBones(aTopBones)
    , mPrevs()
    , mNexts(aNexts)
    , mDone(false)
{
}

void RotateAllBones::modifyValue(const QVector<float>& aNexts)
{
    mNexts = aNexts;
    if (mDone) redo();
}

void RotateAllBones::exec()
{
    for (auto topBone : mTopBones)
    {
        core::Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            mPrevs.push_back(itr.next()->rotate());
        }
    }
    redo();
}

void RotateAllBones::undo()
{
    auto prev = mPrevs.begin();
    for (auto topBone : mTopBones)
    {
        core::Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            if (prev == mPrevs.end()) break;
            itr.next()->setRotate(*prev);
            ++prev;
        }
        topBone->updateWorldTransform();
    }
    mDone = false;
}

void RotateAllBones::redo()
{
    auto next = mNexts.begin();
    for (auto topBone : mTopBones)
    {
        core::Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            if (next == mNexts.end()) break;
            itr.next()->setRotate(*next);
            ++next;
        }
        topBone->updateWorldTransform();
    }
    mDone = true;
}

} // namespace pose
} // namespace ctrl
