#include <utility>
#include "img/ResourceHandle.h"

namespace img
{

ResourceHandle::ResourceHandle()
    : mData()
    , mOriginKeepingCount()
    , mKeepOrigin()
{
}

ResourceHandle::~ResourceHandle()
{
    setOriginKeeping(false);
}

ResourceHandle::ResourceHandle(ResourceData* aData, int* aOriginKeepingCount)
    : mData(aData)
    , mOriginKeepingCount(aOriginKeepingCount)
    , mKeepOrigin()
{
}

ResourceHandle::ResourceHandle(const ResourceHandle& aRhs)
    : mData(aRhs.mData)
    , mOriginKeepingCount(aRhs.mOriginKeepingCount)
    , mKeepOrigin()
{
}

ResourceHandle& ResourceHandle::operator=(const ResourceHandle& aRhs)
{
    const bool keepOrigin = mKeepOrigin;
    setOriginKeeping(false);

    mData = aRhs.mData;
    mOriginKeepingCount = aRhs.mOriginKeepingCount;

    setOriginKeeping(keepOrigin);

    return *this;
}

void ResourceHandle::reset()
{
    const bool keepOrigin = mKeepOrigin;
    setOriginKeeping(false);

    mData.reset();
    mOriginKeepingCount.reset();

    setOriginKeeping(keepOrigin);
}

void ResourceHandle::swapData(ResourceHandle& aRhs)
{
    mData.swap(aRhs.mData);
}

void ResourceHandle::setOriginKeeping(bool aKeepOrigin)
{
    if (!mKeepOrigin && aKeepOrigin)
    {
        mKeepOrigin = aKeepOrigin;
        if (mOriginKeepingCount)
        {
            ++(*mOriginKeepingCount);
        }
    }
    else if (mKeepOrigin && !aKeepOrigin)
    {
        mKeepOrigin = aKeepOrigin;
        if (mOriginKeepingCount)
        {
            XC_ASSERT(*mOriginKeepingCount > 0);
            --(*mOriginKeepingCount);
        }
    }
}

} // namespace img
