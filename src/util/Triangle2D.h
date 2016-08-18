#ifndef UTIL_TRIANGLE2D_H
#define UTIL_TRIANGLE2D_H

#include <QVector2D>
#include "util/Segment2D.h"

namespace util
{

class Triangle2D
{
public:
    Triangle2D(const QVector2D& aV0, const QVector2D& aV1, const QVector2D& aV2)
    {
        v[0] = aV0;
        v[1] = aV1;
        v[2] = aV2;
    }

    Triangle2D(const QVector2D aV[])
    {
        for (int i = 0; i < 3; ++i)
        {
            v[i] = aV[i];
        }
    }

    inline QVector2D dir01() const { return v[1] - v[0]; }
    inline QVector2D dir12() const { return v[2] - v[1]; }
    inline QVector2D dir20() const { return v[0] - v[2]; }

    inline Segment2D seg01() const { return Segment2D(v[0], dir01()); }
    inline Segment2D seg12() const { return Segment2D(v[1], dir12()); }
    inline Segment2D seg20() const { return Segment2D(v[2], dir20()); }

    bool hasFace(float aEps) const;

    void makeSureAnticlockwise();

    QRectF boundingRect() const;

    QVector2D v[3];
};

} // namespace util

#endif // UTIL_TRIANGLE2D_H
