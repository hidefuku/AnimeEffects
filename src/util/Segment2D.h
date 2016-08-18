#ifndef UTIL_SEGMENT2D_H
#define UTIL_SEGMENT2D_H

#include <QVector2D>
#include <QRectF>

namespace util
{

class Segment2D
{
public:
    Segment2D()
        : start()
        , dir()
    {
    }

    Segment2D(const QVector2D& aStart, const QVector2D& aDir)
        : start(aStart)
        , dir(aDir)
    {
    }

    inline QVector2D end() const { return start + dir; }

    QRectF boundingRect() const
    {
        if (dir.x() >= 0)
        {
            if (dir.y() >= 0)
            {
                const QPointF pos(start.toPointF());
                const QSizeF size(dir.x(), dir.y());
                return QRectF(pos, size);
            }
            else
            {
                const QPointF pos(start.x(), start.y() + dir.y());
                const QSizeF size(dir.x(), -dir.y());
                return QRectF(pos, size);
            }
        }
        else
        {
            if (dir.y() >= 0)
            {
                const QPointF pos(start.x() + dir.x(), start.y());
                const QSizeF size(-dir.x(), dir.y());
                return QRectF(pos, size);
            }
            else
            {
                const QPointF pos((start + dir).toPointF());
                const QSizeF size(-dir.x(), -dir.y());
                return QRectF(pos, size);
            }
        }
    }

    QVector2D start;
    QVector2D dir;
};

} // namespace util

#endif // UTIL_SEGMENT2D_H
