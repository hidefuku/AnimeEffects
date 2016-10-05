#include "util/CollDetect.h"
#include "gl/Triangulator.h"

namespace gl
{

Triangulator::Triangulator(const QPoint* aPoints, int aCount)
    : mPoints()
    , mTriangles()
    , mFirstPoint()
    , mIsSuccess(false)
{
    if (aCount < 3) return;
    makeRoundChain<QPoint>(aPoints, aCount);
    mIsSuccess = triangulate();
    if (!mIsSuccess) mTriangles.clear();
}

Triangulator::Triangulator(const QPointF* aPoints, int aCount)
    : mPoints()
    , mTriangles()
    , mFirstPoint()
    , mIsSuccess(false)
{
    if (aCount < 3) return;
    makeRoundChain<QPointF>(aPoints, aCount);
    mIsSuccess = triangulate();
    if (!mIsSuccess) mTriangles.clear();
}

Triangulator::Triangulator(const QPolygonF& aPolygon)
    : mPoints()
    , mTriangles()
    , mFirstPoint()
    , mIsSuccess(false)
{
    if (aPolygon.size() < 3) return;
    makeRoundChain<QPointF>(aPolygon.data(), aPolygon.size());
    mIsSuccess = triangulate();
    if (!mIsSuccess) mTriangles.clear();
}

bool Triangulator::triangulate()
{
    mTriangles.reserve(mPoints.size() * 3);

    int remain = mPoints.size();
    if (remain < 3) return false;

    mFirstPoint = &(mPoints[0]);
    Point* current = mFirstPoint;

    while (1)
    {
        bool foundEar = false;

        while (current != mFirstPoint->prev)
        {
            if (isConvex(*current))
            {
                if (isEar(*current))
                {
                    // push triangle(to anti-clockwise)
                    mTriangles.push_back(gl::Vector2::make(current->next->pos));
                    mTriangles.push_back(gl::Vector2::make(current->pos));
                    mTriangles.push_back(gl::Vector2::make(current->prev->pos));

                    // remove point
                    current->prev->next = current->next;
                    current->next->prev = current->prev;
                    --remain;

                    if (remain >= 3)
                    {
                        // restart
                        mFirstPoint = current->next;
                        current = mFirstPoint;
                        foundEar = true;
                        break;
                    }
                    else
                    {
                        return true; // end
                    }
                }
            }
            current = current->next;
        }
        if (!foundEar)
        {
            return false;
        }
    }
    XC_ASSERT(0); // should not reach to here
    return false;
}

bool Triangulator::isConvex(const Point& aPoint) const
{
    auto a = aPoint.prev->pos;
    auto b = aPoint.pos;
    auto c = aPoint.next->pos;
    return util::CollDetect::getPosSide(util::Segment2D(a, b - a), c) > 0;
}

bool Triangulator::isEar(const Point& aPoint) const
{
    Point* curr = aPoint.next->next;

    while (curr != aPoint.prev)
    {
        if (util::CollDetect::isInside(util::Triangle2D(aPoint.prev->pos, aPoint.pos, aPoint.next->pos), curr->pos))
        {
            return false;
        }
        curr = curr->next;
    }
    return true;
}

} // namespace gl
