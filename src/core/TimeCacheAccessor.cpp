#include "core/TimeCacheAccessor.h"
#include "core/TimeKeyBlender.h"
#include "core/ObjectNode.h"
#include "core/TimeCacheLock.h"
#include "core/TimeKeyExpans.h"

namespace core
{

TimeCacheAccessor::TimeCacheAccessor(
        ObjectNode& aRootNode, TimeCacheLock& aLock,
        const TimeInfo& aTime, bool aUseWorking)
    : mLockRef(aUseWorking ? aLock.working : aLock.current)
    , mUseWorking(aUseWorking)
{
    TimeLine* timeLine = aRootNode.timeLine();
    TimeKeyExpans* expans = nullptr;
    if (timeLine)
    {
        expans = aUseWorking ? &(timeLine->working()) : &(timeLine->current());
    }

    mLockRef.lockForRead();

    if (!timeLine || !expans->hasMasterCache(aTime.frame))
    {
        mLockRef.unlock();
        mLockRef.lockForWrite();

        TimeKeyBlender blender(aRootNode, aUseWorking);
        blender.updateCurrents(&aRootNode, aTime);
    }
}

TimeCacheAccessor::~TimeCacheAccessor()
{
    mLockRef.unlock();
}

const TimeKeyExpans& TimeCacheAccessor::get(const ObjectNode& aNode) const
{
    XC_PTR_ASSERT(aNode.timeLine());
    return mUseWorking ?
                aNode.timeLine()->working() :
                aNode.timeLine()->current();
}

const TimeKeyExpans& TimeCacheAccessor::get(const TimeLine& aLine) const
{
    return mUseWorking ? aLine.working() : aLine.current();
}

TimeKeyExpans& TimeCacheAccessor::get(TimeLine& aLine) const
{
    return mUseWorking ? aLine.working() : aLine.current();
}

} // namespace core
