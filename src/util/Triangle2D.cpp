#include "util/Triangle2D.h"
#include "util/CollDetect.h"

namespace util
{

bool Triangle2D::hasFace(float aEps) const
{
    if (CollDetect::getPerpendicularLength(seg01(), v[2]) < aEps) return false;

    return CollDetect::getPerpendicularLength(seg12(), v[0]) >= aEps;
}

void Triangle2D::makeSureAnticlockwise()
{
    if (CollDetect::getPosSide(seg01(), v[2]) == -1)
    {
        QVector2D temp = v[2];
        v[2] = v[1];
        v[1] = temp;
    }
}

QRectF Triangle2D::boundingRect() const
{
    float l, t, r, b;

    if (v[0].x() <= v[1].x())
    {
        l = std::min(v[0].x(), v[2].x());
        r = std::max(v[1].x(), v[2].x());
    }
    else
    {
        l = std::min(v[1].x(), v[2].x());
        r = std::max(v[0].x(), v[2].x());
    }
    if (v[0].y() <= v[1].y())
    {
        t = std::min(v[0].y(), v[2].y());
        b = std::max(v[1].y(), v[2].y());
    }
    else
    {
        t = std::min(v[1].y(), v[2].y());
        b = std::max(v[0].y(), v[2].y());
    }
    return QRectF(l, t, r - l, b - t);
}

} // namespace util

