#ifndef UTIL_CIRCLE
#define UTIL_CIRCLE

#include <QPointF>
#include <QVector2D>

namespace util
{
class Circle
{
public:
    Circle()
        : mCenter()
        , mRadius()
    {
    }

    Circle(const QVector2D& aCenter, float aRadius)
        : mCenter(aCenter)
        , mRadius(aRadius)
    {
    }

    Circle(const QPointF& aCenter, float aRadius)
        : mCenter(aCenter)
        , mRadius(aRadius)
    {
    }

    void setCenter(const QVector2D& aCenter) { mCenter = aCenter; }
    void setRadius(float aRadius) { mRadius = aRadius; }

    const QVector2D& center() const { return mCenter; }
    float radius() const { return mRadius; }

    bool isInside(const QVector2D& aPoint) const
    {
        return (mCenter - aPoint).lengthSquared() <= mRadius * mRadius;
    }

    bool isInside(const QPointF& aPoint) const
    {
        return (mCenter - QVector2D(aPoint)).lengthSquared() <= mRadius * mRadius;
    }

private:
    QVector2D mCenter;
    float mRadius;
};
}

#endif // UTIL_CIRCLE

