#ifndef UTIL_TRIANGLE2DPOS_H
#define UTIL_TRIANGLE2DPOS_H

#include <array>
#include "util/Triangle2D.h"

namespace util
{

class Triangle2DPos
{
public:
    static Triangle2DPos make(const Triangle2D& aTriangle, const QVector2D& aPoint);

    Triangle2DPos();
    Triangle2DPos(int aIndex, float aFirst, float aSecond);

    bool isValid() const { return mIndex >= 0; }
    QVector2D get(const Triangle2D& aTriangle) const;
    QVector2D get(const std::array<QVector2D, 3>& aTriangle) const;

private:
    int mIndex;
    float mFirst;
    float mSecond;
};

} // namespace util

#endif // UTIL_TRIANGLE2DPOS_H
