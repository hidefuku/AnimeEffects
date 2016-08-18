#ifndef CORE_TIMECACHEACCESSOR_H
#define CORE_TIMECACHEACCESSOR_H

#include <QReadWriteLock>
namespace core { class ObjectNode; }
namespace core { class TimeLine; }
namespace core { class TimeInfo; }
namespace core { class TimeCacheLock; }
namespace core { class TimeKeyExpans; }

namespace core
{

class TimeCacheAccessor
{
    QReadWriteLock& mLockRef;
    bool mUseWorking;

public:
    TimeCacheAccessor(
            ObjectNode& aRootNode, TimeCacheLock& aLock,
            const TimeInfo& aTime, bool aUseWorking);
    ~TimeCacheAccessor();
    const TimeKeyExpans& get(const ObjectNode& aNode) const;
    const TimeKeyExpans& get(const TimeLine& aLine) const;
    TimeKeyExpans& get(TimeLine& aLine) const;
};

} // namespace core

#endif // CORE_TIMECACHEACCESSOR_H
