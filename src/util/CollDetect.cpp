#include <limits>
#include "util/CollDetect.h"

namespace util
{

float CollDetect::getMinDistanceSquared(const Segment2D& aSegment, const QVector2D& aPoint)
{
    QVector2D s2p = aPoint - aSegment.start;

    float ratio = QVector2D::dotProduct(aSegment.dir, s2p) / aSegment.dir.lengthSquared();

    if (ratio < 0.0f)
    {
        return s2p.lengthSquared();
    }
    else if (ratio > 1.0f)
    {
        return (s2p - aSegment.dir).lengthSquared();
    }
    else
    {
        return (s2p - ratio * aSegment.dir).lengthSquared();
    }
}

float CollDetect::getPerpendicularLength(const Segment2D& aSegment, const QVector2D& aPoint)
{
    QVector2D s2p = aPoint - aSegment.start;

    float ratio = QVector2D::dotProduct(aSegment.dir, s2p) / aSegment.dir.lengthSquared();
    return (s2p - ratio * aSegment.dir).length();
}

float CollDetect::getRawSegmentRate(const util::Segment2D& aSegment, const QVector2D& aPoint)
{
    QVector2D s2p = aPoint - aSegment.start;
    float ratio = QVector2D::dotProduct(aSegment.dir, s2p) / aSegment.dir.lengthSquared();
    return ratio;
}

QVector2D CollDetect::getPosOnSegment(const Segment2D& aSegment, const QVector2D& aPoint)
{
    QVector2D s2p = aPoint - aSegment.start;

    float ratio = QVector2D::dotProduct(aSegment.dir, s2p) / aSegment.dir.lengthSquared();
    ratio = xc_clamp(ratio, 0.0f, 1.0f);
    return aSegment.start + ratio * aSegment.dir;
}

QVector2D CollDetect::getPosOnLine(const Segment2D& aSegment, const QVector2D& aPoint)
{
    QVector2D s2p = aPoint - aSegment.start;

    float ratio = QVector2D::dotProduct(aSegment.dir, s2p) / aSegment.dir.lengthSquared();
    return aSegment.start + ratio * aSegment.dir;
}

// left is 1, right is -1, on the line is 0
int CollDetect::getPosSide(const Segment2D& aSegment, const QVector2D& aPoint)
{
    QVector2D s2p = aPoint - aSegment.start;

    float cross = getCross(aSegment.dir, s2p);
    if (cross > 0) return 1;
    else if (cross < 0) return -1;
    else return 0;
}

// triangle vertex must be anticlockwise rotation
bool CollDetect::isInside(const Triangle2D& aTriangle, const QVector2D& aPoint)
{
    if (getPosSide(aTriangle.seg01(), aPoint) == -1)
    {
        return false;
    }
    else if (getPosSide(aTriangle.seg12(), aPoint) == -1)
    {
        return false;
    }
    else if (getPosSide(aTriangle.seg20(), aPoint) == -1)
    {
        return false;
    }
    return true;
}

bool CollDetect::isInside(const QVector2D* aConvexPolygon, int aVertexNum, const QVector2D& aPoint)
{
    for (int i = 0; i < aVertexNum; ++i)
    {
        int k = (i + 1) % aVertexNum;
        if (getPosSide(Segment2D(aConvexPolygon[i], aConvexPolygon[k] - aConvexPolygon[i]), aPoint) == -1)
        {
            return false;
        }
    }
    return true;
}

bool CollDetect::isInside(const QPointF* aConvexPolygon, int aVertexNum, const QPointF& aPoint)
{
    for (int i = 0; i < aVertexNum; ++i)
    {
        int k = (i + 1) % aVertexNum;
        if (getPosSide(Segment2D(QVector2D(aConvexPolygon[i]), QVector2D(aConvexPolygon[k] - aConvexPolygon[i])), QVector2D(aPoint)) == -1)
        {
            return false;
        }
    }
    return true;
}

bool CollDetect::rayIntersectsToPolygonEdge(
        const QVector2D& a, const QVector2D& b, const QVector2D& p)
{
    auto kEps = std::numeric_limits<float>::epsilon();
    auto kMin = std::numeric_limits<float>::min();
    auto kMax = std::numeric_limits<float>::max();

    if (p.y() == a.y() || p.y() == b.y())
    {
        return rayIntersectsToPolygonEdge(a, b, QVector2D(p.x(), p.y() + kEps));
    }

    if (p.y() > b.y() || p.y() < a.y() || p.x() > std::max(a.x(), b.x()))
    {
        return false;
    }

    if (p.x() < std::min(a.x(), b.x()))
    {
        return true;
    }

    auto blue = std::abs(a.x() - p.x()) > kMin ? (p.y() - a.y()) / (p.x() - a.x()) : kMax;
    auto red  = std::abs(a.x() - b.x()) > kMin ? (b.y() - a.y()) / (b.x() - a.x()) : kMax;
    return blue >= red;
}

bool CollDetect::isInsideOfPolygon(const util::ArrayBlock<QVector2D>& aPolygon, const QVector2D& aPoint)
{
    int count = 0;

    for (int i = 0; i < aPolygon.count() - 1; ++i)
    {
        QVector2D a = aPolygon[i];
        QVector2D b = aPolygon[i + 1];
        if (a.y() > b.y()) std::swap(a, b);

        if (rayIntersectsToPolygonEdge(a, b, aPoint))
        {
            ++count;
        }
    }
    return count % 2 != 0;
}

QVector2D CollDetect::getTriangleCenter(const Triangle2D& aTriangle)
{
    return (aTriangle.v[0] + aTriangle.v[1] + aTriangle.v[2]) / 3.0f;
}

bool CollDetect::intersects(const Segment2D& aSeg0, const Segment2D& aSeg1)
{
    float cross = getCross(aSeg0.dir, aSeg1.dir);
    if (cross == 0.0f)
    {
        return false;
    }

    QVector2D s2s = aSeg1.start - aSeg0.start;

    float crossA = getCross(s2s, aSeg0.dir);
    float crossB = getCross(s2s, aSeg1.dir);

    float tA = crossB / cross;
    float tB = crossA / cross;

    if (tA < 0.0f || 1.0f < tA || tB < 0.0f || 1.0f < tB)
    {
        return false;
    }

    return true;
}

std::pair<bool, QVector2D> CollDetect::getIntersection(
        const Segment2D& aSeg0, const Segment2D& aSeg1)
{
    float cross = getCross(aSeg0.dir, aSeg1.dir);
    if (cross == 0.0f)
    {
        return std::pair<bool, QVector2D>(false, QVector2D());
    }

    QVector2D s2s = aSeg1.start - aSeg0.start;

    float crossA = getCross(s2s, aSeg0.dir);
    float crossB = getCross(s2s, aSeg1.dir);

    float tA = crossB / cross;
    float tB = crossA / cross;

    if (tA < 0.0f || 1.0f < tA || tB < 0.0f || 1.0f < tB)
    {
        return std::pair<bool, QVector2D>(false, QVector2D());
    }

    return std::pair<bool, QVector2D>(true, aSeg0.start + aSeg0.dir * tA);
}

} // namespace util
