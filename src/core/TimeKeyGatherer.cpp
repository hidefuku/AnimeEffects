#include "core/TimeKeyGatherer.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
TimeKeyGatherer::Point::Point()
    : key()
    , frame()
    , relativeFrame()
    , looped()
{
}

//-------------------------------------------------------------------------------------------------
TimeKey* TimeKeyGatherer::findLastKey(const TimeLine::MapType& aMap, Frame aFrame)
{
    if (!aMap.isEmpty())
    {
        auto itr = aMap.upperBound(aFrame.get());
        if (itr != aMap.begin())
        {
            --itr;
            return itr.value();
        }
    }
    return nullptr;
}

//-------------------------------------------------------------------------------------------------
TimeKeyGatherer::TimeKeyGatherer()
    : mPoints()
    , mFrame()
{
}

TimeKeyGatherer::TimeKeyGatherer(
        const TimeLine::MapType& aMap,
        const TimeInfo& aTimeInfo,
        ForceType aForceType,
        TimeKey* aAssignedParent)
    : mPoints()
    , mFrame(aTimeInfo.frame.get())
    , mForceType(aForceType)
    , mParent()
{
    const bool forced =
            mForceType == ForceType_SameParent ||
            mForceType == ForceType_AssignedParent;

    // forced assigned parent
    if (mForceType == ForceType_AssignedParent && aAssignedParent)
    {
        mParent = aAssignedParent;
    }

    if (aMap.isEmpty())
    {
        return;
    }

    if (!aTimeInfo.frame.hasFraction() && aMap.contains(mFrame))
    {
        TimeKey* key = aMap.value(mFrame);

        if (mForceType != ForceType_AssignedParent || mParent == key->parent())
        {
            mParent = key->parent();
            mPoints[1].key = key;
            mPoints[1].frame = mFrame;
            mPoints[1].relativeFrame = 0.0f;
            mPoints[1].looped = false;
            return;
        }
    }

    const int count = aMap.size();
    const int frameMax = aTimeInfo.frameMax;
    const int loop = aTimeInfo.loop;

    auto forward = aMap.upperBound(mFrame);
    auto backward = forward;

    // backward iterate
    do
    {
        Frame baseFrame = aTimeInfo.frame;
        bool isLooped = false;

        if (backward == aMap.begin())
        {
            if (loop && count > 1)
            {
                backward = aMap.end();
                baseFrame.add(frameMax + 1);
                isLooped = true;
            }
            else
            {
                break;
            }
        }
        --backward;

        // forced assigned parent, and mismatch
        if (mForceType == ForceType_AssignedParent &&
                backward.value()->parent() != mParent)
        {
            break;
        }
        else if (mForceType == ForceType_SameParent)
        {// forced same parent
            mParent = backward.value()->parent();
        }

        const int frame = backward.key();
        mPoints[1].key = backward.value();
        mPoints[1].frame = frame;
        mPoints[1].relativeFrame = frame - baseFrame.getDecimal();
        mPoints[1].looped = isLooped;
        XC_PTR_ASSERT(mPoints[1].key);

        for (int i = 0; i >= 0; --i)
        {
            if (backward == aMap.begin())
            {
                break;
            }
            --backward;

            if (forced && backward.value()->parent() != mParent)
            {
                break;
            }

            const int frame = backward.key();
            mPoints[i].key = backward.value();
            mPoints[i].frame = frame;
            mPoints[i].relativeFrame = frame - baseFrame.getDecimal();
            mPoints[i].looped = isLooped;
            XC_PTR_ASSERT(mPoints[i].key);
        }
    }
    while(0);

    // forward iterate
    do
    {
        Frame baseFrame = aTimeInfo.frame;
        bool isLooped = false;

        if (forward == aMap.end())
        {
            if (loop && count > 1)
            {
                forward = aMap.begin();
                baseFrame.add(-frameMax - 1);
                isLooped = true;
            }
            else
            {
                break;
            }
        }

        if (forced && forward.value()->parent() != mParent)
        {
            break;
        }

        const int frame = forward.key();
        mPoints[2].key = forward.value();
        mPoints[2].frame = frame;
        mPoints[2].relativeFrame = frame - baseFrame.getDecimal();
        mPoints[2].looped = isLooped;
        ++forward;

        for (int i = 3; i < 4; ++i)
        {
            if (forward == aMap.end())
            {
                break;
            }

            if (forced && forward.value()->parent() != mParent)
            {
                break;
            }

            const int frame = forward.key();
            mPoints[i].key = forward.value();
            mPoints[i].frame = frame;
            mPoints[i].relativeFrame = frame - baseFrame.getDecimal();
            mPoints[i].looped = isLooped;
            ++forward;

            XC_PTR_ASSERT(mPoints[i].key);
        }
    }
    while(0);
}

} // namespace core
