#include "util/Triangle2DPos.h"
#include "util/CollDetect.h"

namespace util
{


Triangle2DPos Triangle2DPos::make(const util::Triangle2D& aTriangle, const QVector2D& aPoint)
{
    int farIndex = -1;
    float maxLengthSq = 0;
    QVector2D dir[3];
    for (int i = 0; i < 3; ++i)
    {
        dir[i] = aPoint - aTriangle.v[i];
        auto lengthSq = dir[i].lengthSquared();
        if (lengthSq > maxLengthSq)
        {
            farIndex = i;
            maxLengthSq = lengthSq;
        }
    }

    if (farIndex == -1)
    {
        return Triangle2DPos();
    }

    const int firstIndex = (farIndex + 1) % 3;
    const QVector2D s0 = aTriangle.v[firstIndex];
    const QVector2D e0 = aTriangle.v[(farIndex + 2) % 3];
    const QVector2D s1 = aTriangle.v[farIndex];
    const QVector2D v0 = e0 - s0;
    const QVector2D v1 = dir[farIndex];
    const QVector2D v = s1 - s0;

    const float cross = util::CollDetect::getCross(v0, v1);
    if (cross != 0.0f)
    {
        const float cross1 = util::CollDetect::getCross(v, v1);
        const float t0 = cross1 / cross;

        if (xc_contains(t0, 0.0f, 1.0f))
        {
            const QVector2D x = s0 + v0 * t0;
            const QVector2D v2 = s1 - x;
            const QVector2D v3 = aPoint - x;
            const float v2len = v2.length();

            if (v2len > 0 && QVector2D::dotProduct(v2, v3) >= 0.0f)
            {
                const float t1 = v3.length() / v2len;

                if (xc_contains(t1, 0.0f, 1.0f))
                {
                    return Triangle2DPos(firstIndex, 1.0f - t0, 1.0f - t1);
                }
            }
        }
    }
    return Triangle2DPos();
}

Triangle2DPos::Triangle2DPos()
    : mIndex(-1)
    , mFirst()
    , mSecond()
{
}

Triangle2DPos::Triangle2DPos(int aIndex, float aFirst, float aSecond)
    : mIndex(aIndex)
    , mFirst(aFirst)
    , mSecond(aSecond)
{
    XC_ASSERT(0 <= mIndex && mIndex < 3);
    XC_ASSERT(xc_contains(aFirst, 0.0f, 1.0f));
    XC_ASSERT(xc_contains(aSecond, 0.0f, 1.0f));
}

QVector2D Triangle2DPos::get(const Triangle2D& aTriangle) const
{
    XC_ASSERT(isValid());
    const QVector2D first = aTriangle.v[mIndex] * mFirst +
            aTriangle.v[(mIndex + 1) % 3] * (1.0f - mFirst);

    return first * mSecond + aTriangle.v[(mIndex + 2) % 3] * (1.0f - mSecond);
}

QVector2D Triangle2DPos::get(const std::array<QVector2D, 3>& aTriangle) const
{
    XC_ASSERT(isValid());
    const QVector2D first = aTriangle[mIndex] * mFirst +
            aTriangle[(mIndex + 1) % 3] * (1.0f - mFirst);

    return first * mSecond + aTriangle[(mIndex + 2) % 3] * (1.0f - mSecond);
}

} // namespace util

