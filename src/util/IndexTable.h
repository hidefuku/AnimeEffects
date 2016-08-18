#ifndef UTIL_INDEXTABLE_H
#define UTIL_INDEXTABLE_H

#include <QScopedArrayPointer>
#include "XC.h"

namespace util
{

class IndexTable
{
public:
    static const uint32 kInvalidIndex;

    IndexTable();

    explicit operator bool() const { return (bool)mTable.data(); }
    void alloc(int aWidth, int aHeight);
    void free();
    void clear();

    void setIndex(int aX, int aY, uint32 aIndex) { mTable[aX + aY * mWidth] = aIndex; }
    uint32 index(int aX, int aY) const { return mTable[aX + aY * mWidth]; }

    uint32* data() { return mTable.data(); }
    const uint32* data() const { return mTable.data(); }

    int width() const { return mWidth; }
    int height() const { return mHeight; }
    int count() const { return mCount; }

private:
    QScopedArrayPointer<uint32> mTable;
    int mWidth;
    int mHeight;
    int mCount;
};

} // namespace util

#endif // UTIL_INDEXTABLE_H
