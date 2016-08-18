#ifndef CORE_TIMEKEYGATHERER_H
#define CORE_TIMEKEYGATHERER_H

#include <array>
#include "core/TimeLine.h"
#include "core/TimeInfo.h"
#include "XC.h"

namespace core
{

class TimeKeyGatherer
{
public:
    enum ForceType
    {
        ForceType_None,
        ForceType_SameParent,
        ForceType_AssignedParent,
    };

    struct Point
    {
        Point();
        explicit operator bool() const { return key != NULL; }
        TimeKey* key;
        int frame;
        float relativeFrame;
        bool looped;
    };

    static TimeKey* findLastKey(const TimeLine::MapType& aMap, Frame aFrame);

    TimeKeyGatherer();
    TimeKeyGatherer(
            const TimeLine::MapType& aMap,
            const TimeInfo& aTimeInfo,
            ForceType aForceType = ForceType_None,
            TimeKey* aAssignedParent = nullptr);

    bool hasSameFrame() const { return (bool)point(0) && point(0).frame == mFrame; }
    bool isEmpty() const { return !(bool)point(0) && !(bool)point(1); }
    bool isSingle() const { return !isEmpty() && !isSandwiched(); }
    bool isSandwiched() const { return (bool)point(0) && (bool)point(1); }

    //@param [in] aIndex (-1...2)
    const Point& point(int aIndex) const { return mPoints[aIndex + 1]; }
    const Point& singlePoint() const { XC_ASSERT(isSingle()); return (bool)point(0) ? point(0) : point(1); }

    TimeKey* parent() const { return mParent; }
private:
    std::array<Point, 4> mPoints;
    const int mFrame;
    ForceType mForceType;
    TimeKey* mParent;
};

} // namespace core

#endif // CORE_TIMEKEYGATHERER_H
