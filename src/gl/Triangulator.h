#ifndef GL_TRIANGULATOR_H
#define GL_TRIANGULATOR_H

#include <QPolygonF>
#include <QVector2D>
#include <QVector>
#include "XC.h"
#include "gl/Vector2.h"

namespace gl
{

// south east coordinate
// triangulate a simple clockwise polygon by ear clipping algorithm.
class Triangulator
{
public:
    Triangulator(const QPoint* aPoints, int aCount);
    Triangulator(const QPointF* aPoints, int aCount);
    Triangulator(const QPolygonF& aPolygon);
    explicit operator bool() const { return mIsSuccess; }
    const QVector<gl::Vector2>& triangles() const { return mTriangles; }

private:
    struct Point
    {
        Point() : pos(), prev(), next() {}
        QVector2D pos;
        Point* prev;
        Point* next;
    };

    template<typename tPointType> void makeRoundChain(const tPointType* aPoints, int aCount);

    bool triangulate();
    bool isConvex(const Point& aPoint) const;
    bool isEar(const Point& aPoint) const;

    QVector<Point> mPoints;
    QVector<gl::Vector2> mTriangles;
    Point* mFirstPoint;
    bool mIsSuccess;
};

template<typename tPointType>
void Triangulator::makeRoundChain(const tPointType* aPoints, int aCount)
{
    mPoints.resize(aCount);

    // make round chain
    Point* prev = &(mPoints[aCount - 1]);
    for (int i = 0; i < aCount; ++i)
    {
        auto& curr = mPoints[i];
        curr.pos = QVector2D(aPoints[i]);
        prev->next = &curr;
        curr.prev = prev;
        prev = &curr;
    }
}

} // namespace gl

#endif // GL_TRIANGULATOR_H
