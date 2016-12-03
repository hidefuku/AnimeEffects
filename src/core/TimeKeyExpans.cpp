#include "core/TimeKeyExpans.h"

namespace core
{

TimeKeyExpans::TimeKeyExpans()
    : mMasterCache()
    , mKeyCaches()
    , mSRT()
    , mOpa()
    , mWorldOpacity(1.0f)
    , mDepth()
    , mWorldDepth()
    , mBone()
    , mPose()
    , mPoseParent()
    , mPosePalette()
    , mAreaMeshKey()
    , mFFD()
    , mFFDMesh()
    , mFFDMeshParent()
    , mAreaImageKey()
    , mImageOffset()
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

//-------------------------------------------------------------------------------------------------
const gl::Texture* TimeKeyExpans::areaTexture() const
{
    return mAreaImageKey ? &(mAreaImageKey->cache().texture()) : nullptr;
}

img::BlendMode TimeKeyExpans::blendMode() const
{
    return mAreaImageKey ? mAreaImageKey->data().blendMode() : img::BlendMode_Normal;
}

} // namespace core
