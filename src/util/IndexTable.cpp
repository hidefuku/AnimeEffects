#include "util/IndexTable.h"

namespace util
{
const uint32 IndexTable::kInvalidIndex = static_cast<uint32>(-1);

IndexTable::IndexTable()
    : mTable()
    , mWidth()
    , mHeight()
    , mCount()
{
}

void IndexTable::alloc(int aWidth, int aHeight)
{
    mWidth = aWidth;
    mHeight = aHeight;

    const int prevCount = mCount;
    mCount = aWidth * aHeight;

    if (prevCount != mCount)
    {
        if (mCount)
        {
            mTable.reset(new uint32[mCount]);
        }
        else
        {
            mTable.reset();
        }
    }
}

void IndexTable::free()
{
    alloc(0, 0);
}

void IndexTable::clear()
{
    for (int i = 0; i < mCount; ++i)
    {
        mTable[i] = kInvalidIndex;
    }
}

} // namespace util
