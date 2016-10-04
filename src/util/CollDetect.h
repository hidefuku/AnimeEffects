#ifndef UTIL_COLLDETECT_H
#define UTIL_COLLDETECT_H

#include <QVector2D>
#include "util/Segment2D.h"
#include "util/Triangle2D.h"
#include "util/ArrayBlock.h"

namespace util
{
// north east coordinate
class CollDetect
{
public:
    static inline float getCross(const QVector2D& aV0, const QVector2D& aV1)
    {
        return aV0.x() * aV1.y() - aV0.y() * aV1.x();
    }

    static float getMinDistanceSquared(const Segment2D& aSegment, const QVector2D& aPoint);

    static float getPerpendicularLength(const Segment2D& aSegment, const QVector2D& aPoint);

    static float getRawSegmentRate(const util::Segment2D& aSegment, const QVector2D& aPoint);

    static QVector2D getPosOnSegment(const Segment2D& aSegment, const QVector2D& aPoint);
    static QVector2D getPosOnLine(const Segment2D& aSegment, const QVector2D& aPoint);

    // left is 1, right is -1, on the line is 0
    static int getPosSide(const Segment2D& aSegment, const QVector2D& aPoint);

    // triangle vertex must be anticlockwise rotation
    static bool isInside(const Triangle2D& aTriangle, const QVector2D& aPoint);

    // convex polygon vertex must be anticlockwise rotation
    static bool isInside(const QVector2D* aConvexPolygon, int aVertexNum, const QVector2D& aPoint);
    static bool isInside(const QPointF* aConvexPolygon, int aVertexNum, const QPointF& aPoint);

    // polygon vertex must be anticlockwise rotation
    static bool isInsideOfPolygon(const util::ArrayBlock<QVector2D>& aPolygon, const QVector2D& aPoint);

    static QVector2D getTriangleCenter(const Triangle2D& aTriangle);

    static bool intersects(const Segment2D& aSeg0, const Segment2D& aSeg1);

    static std::pair<bool, QVector2D> getIntersection(
            const Segment2D& aSeg0, const Segment2D& aSeg1);

private:
    static bool rayIntersectsToPolygonEdge(
            const QVector2D& aTop, const QVector2D& aBottom, const QVector2D& aPoint);

    CollDetect() {}
};

} // namespace util

#endif // UTIL_COLLDETECT_H
