#ifndef CORE_TIMEKEYPOS
#define CORE_TIMEKEYPOS

#include "core/TimeLine.h"

namespace core
{

class TimeKeyPos
{
public:
    TimeKeyPos()
        : mLine()
        , mType()
        , mIndex()
    {
    }

    TimeKeyPos(TimeLine& aLine, TimeKeyType aType, int aIndex)
        : mLine(&aLine)
        , mType(aType)
        , mIndex(aIndex)
    {
    }

    void setLine(TimeLine* aLine) { mLine = aLine; }
    void setType(TimeKeyType aType) { mType = aType; }
    void setIndex(int aIndex) { mIndex = aIndex; }

    bool isNull() const { return mLine == NULL; }

    TimeLine* line() { return mLine; }
    const TimeLine* line() const { return mLine; }

    TimeKeyType type() const { return mType; }
    int index() const { return mIndex; }

    bool isExist() const
    {
        if (mLine) return mLine->map(mType).contains(mIndex);
        return false;
    }

    TimeKey* key()
    {
        if (mLine) return mLine->map(mType).value(mIndex);
        return nullptr;
    }

    const TimeLine::MapType& map() const
    {
        XC_PTR_ASSERT(mLine);
        return mLine->map(mType);
    }

private:
    TimeLine* mLine;
    TimeKeyType mType;
    int mIndex;
};

}

#endif // CORE_TIMEKEYPOS

