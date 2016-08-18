#ifndef UTIL_ARRAYBLOCK
#define UTIL_ARRAYBLOCK

#include "XC.h"

namespace util
{

template<typename tObj>
class ArrayBlock
{
public:
    ArrayBlock()
        : mArray()
        , mCount()
    {
    }

    ArrayBlock(tObj* aArray, int aCount)
        : mArray(aArray)
        , mCount(aCount)
    {
        XC_ASSERT(aCount > 0);
    }

    explicit operator bool() const
    {
        return mArray && mCount > 0;
    }

    tObj& operator [](int aIndex)
    {
        return mArray[aIndex];
    }

    const tObj& operator [](int aIndex) const
    {
        return mArray[aIndex];
    }

    tObj& at(int aIndex)
    {
        XC_PTR_ASSERT(mArray);
        XC_ASSERT(0 <= aIndex && aIndex < mCount);
        return mArray[aIndex];
    }

    const tObj& at(int aIndex) const
    {
        XC_PTR_ASSERT(mArray);
        XC_ASSERT(0 <= aIndex && aIndex < mCount);
        return mArray[aIndex];
    }

    tObj* array()
    {
        return mArray;
    }

    const tObj* array() const
    {
        return mArray;
    }

    int count() const
    {
        return mCount;
    }

private:
    tObj* mArray;
    int mCount;
};

} // namespace util

#endif // UTIL_ARRAYBLOCK

