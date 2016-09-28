#include "core/TimeKeyExpans.h"

namespace core
{

TimeKeyExpans::TimeKeyExpans()
    : mMasterCache()
    , mKeyCaches()
    , mSRT()
    , mOpa()
    , mWorldOpacity(1.0f)
    , mAreaBone()
    , mBoneInfluence()
    , mOuterMtx()
    , mInnerMtx()
    , mBoneParent()
    , mBindingRoot()
    , mBinderBoneIndex(-1)
    , mBindingMtx()
    , mIsUnderOfBinding()
    , mIsAffectedByBinding()
    , mPose()
    , mPoseParent()
    , mPosePalette()
    , mAreaMesh()
    , mFFD()
{
    clearCaches();
}

void TimeKeyExpans::setMasterCache(Frame aFrame)
{
    mMasterCache = aFrame;
}

bool TimeKeyExpans::hasMasterCache(Frame aFrame) const
{
    return mMasterCache >= 0 && mMasterCache == aFrame;
}

void TimeKeyExpans::clearMasterCache()
{
    mMasterCache.set(-1);
}

void TimeKeyExpans::setKeyCache(TimeKeyType aType, Frame aFrame)
{
    mKeyCaches[aType] = aFrame;
}

bool TimeKeyExpans::hasKeyCache(TimeKeyType aType, Frame aFrame) const
{
    return aFrame >= 0 && mKeyCaches[aType] == aFrame;
}

void TimeKeyExpans::clearCaches()
{
    mMasterCache.set(-1);

    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        mKeyCaches[i].set(-1);
    }
    mSRT.clearSplineCache();
}

} // namespace core
