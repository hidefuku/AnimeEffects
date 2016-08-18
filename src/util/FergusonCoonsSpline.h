#ifndef UTIL_FERGUSONCOONSSPLINE
#define UTIL_FERGUSONCOONSSPLINE

#include "util/MathUtil.h"

namespace util
{

template <typename tVec>
class FergusonCoonsSpline
{
    enum { kSplitCount = 8 }; // a power of 2
public:
    FergusonCoonsSpline()
        : mP0()
        , mP1()
        , mV0()
        , mV1()
        , mLength()
    {
        mLinearize[0] = 0.0f;
    }

    void set(const tVec& p0, const tVec& p1, const tVec& v0, const tVec& v1)
    {
        mP0 = p0;
        mP1 = p1;
        mV0 = v0;
        mV1 = v1;

        mLength = 0.0f;
        mLinearize[0] = 0.0f;
        tVec current = mP0;
        for (int i = 1; i <= kSplitCount; ++i)
        {
            tVec next = get((float)(i) / kSplitCount);
            mLength += (next - current).length();
            mLinearize[i] = mLength;
            current = next;
        }
    }

    // ferguson coons fomula
    tVec get(float t) const
    {
        return (2.0f * mP0 - 2.0f * mP1 + mV0 + mV1) * t * t * t +
                (-3.0f * mP0 + 3.0f * mP1 - 2.0f * mV0 - mV1) * t * t + mV0 * t + mP0;
    }

    tVec getByLinear(float t) const
    {
        const float tlen = t * mLength;

        int a = 0;
        int b = kSplitCount >> 1;

        while (b >= 1)
        {
            if (tlen >= mLinearize[a + b]) a += b;
            b >>= 1;
        }
        const float prev = mLinearize[a];
        const float range = mLinearize[a + 1] - prev;

        if (range < 0.001f)
        {
            return get(a / kSplitCount);
        }

        const float offset = (tlen - prev) / range;
        return get(((float)a + offset) / kSplitCount);
    }

    float length() const
    {
        return mLength;
    }

private:
    tVec mP0;
    tVec mP1;
    tVec mV0;
    tVec mV1;
    float mLinearize[kSplitCount + 1];
    float mLength;
};

} // namespace util

#endif // UTIL_FERGUSONCOONSSPLINE

