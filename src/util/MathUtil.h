#ifndef UTIL_MATHUTIL_H
#define UTIL_MATHUTIL_H

#include <QtMath>
#include <QVector2D>
#include <QVector3D>
#include <QRect>
#include <math.h>
#include "util/Segment2D.h"

namespace util
{
// north east coordinate
class MathUtil
{
public:
    static QVector2D getCenter(const QRect& aRect)
    {
        return QVector2D(aRect.left() + aRect.width() * 0.5f,
                         aRect.top() + aRect.height() * 0.5f);
    }

    static QVector2D getCenterOffset(const QRect& aRect)
    {
        return QVector2D(aRect.width() * 0.5f, aRect.height() * 0.5f);
    }

    static QVector3D getCenterOffset3D(const QRect& aRect)
    {
        return QVector3D(aRect.width() * 0.5f, aRect.height() * 0.5f, 0.0f);
    }

    static float getRadianFromDegree(float aDegree)
    {
        return static_cast<float>((static_cast<double>(aDegree) * M_PI) / 180.0);
    }

    static float getDegreeFromRadian(float aRadian)
    {
        return static_cast<float>((static_cast<double>(aRadian) * 180.0) / M_PI);
    }

    static float normalizeAngleRad(float aAngle)
    {
        static const double kPi2 = 2.0 * M_PI;
        const int round = static_cast<int>(static_cast<double>(aAngle) / kPi2);
        return (aAngle >= 0.0f) ?
                    static_cast<float>(static_cast<double>(aAngle) - kPi2 * round) :
                    static_cast<float>(static_cast<double>(aAngle) - kPi2 * (round - 1));
    }

    static float normalizeAngleDeg(float aAngle)
    {
        const int round = static_cast<int>(aAngle / 360.0f);
        return (aAngle >= 0.0f) ?
                    aAngle - 360.0f * round :
                    aAngle - 360.0f * (round - 1);
    }

    static int normalizeAngleDeg(int aAngle)
    {
        const int round = aAngle / 360;
        return (aAngle >= 0) ?
                    aAngle - 360 * round :
                    aAngle - 360 * (round - 1);
    }

    static float normalizeSignedAngleRad(float aAngle)
    {
        double angle = static_cast<double>(normalizeAngleRad(aAngle));
        if (angle > M_PI) angle -= 2.0 * M_PI;
        return static_cast<float>(angle);
    }

    static float getAngleRad(const QVector2D& aVec)
    {
        if (aVec.isNull()) return 0.0f;
        return static_cast<float>(qAtan2(static_cast<double>(aVec.y()), static_cast<double>(aVec.x())));
    }

    static float getAngleDeg(const QVector2D& aVec)
    {
        return getDegreeFromRadian(getAngleRad(aVec));
    }

    static QVector2D getVectorFromPolarCoord(float aLength, float aAngleRad)
    {
        return QVector2D(
                    static_cast<float>(qCos(static_cast<double>(aAngleRad))),
                    static_cast<float>(qSin(static_cast<double>(aAngleRad))) * aLength
                    );
    }

    static float getAngleDifferenceRad(float aAngleFrom, float aAngleTo)
    {
        float diffs = (aAngleTo - aAngleFrom);
        return normalizeSignedAngleRad(diffs);
    }

    static float getAngleDifferenceRad(const QVector2D& aFrom, const QVector2D& aTo)
    {
        return getAngleDifferenceRad(getAngleRad(aFrom), getAngleRad(aTo));
    }

    static float getAngleDifferenceDeg(const QVector2D& aFrom, const QVector2D& aTo)
    {
        return getDegreeFromRadian(getAngleDifferenceRad(aFrom, aTo));
    }

    static float getClockwiseRotationRad(const QVector2D& aFrom, const QVector2D& aTo);

    static QVector2D blendVectorByClockwiseRotation(
            const QVector2D& aFrom, const QVector2D& aTo, float aRate = 0.5f);

    static QVector2D blendVectorByAntiClockwiseRotation(
            const QVector2D& aFrom, const QVector2D& aTo, float aRate = 0.5f)
    {
        return blendVectorByClockwiseRotation(aTo, aFrom, 1.0f - aRate);
    }

    static QVector2D getRotateVectorRad(const QVector2D& aVec, float aRotate);

    static QPointF getRotateVectorRad(const QPointF& aPoint, float aRotate);

    static QVector2D getRotateVector90Deg(const QVector2D& aVec)
    {
        return QVector2D(-aVec.y(), aVec.x());
    }


    static QVector2D getAxisInversed(const QVector2D& aNormAxis, const QVector2D& aVec);

    static QVector3D getAxisInversed(const QVector3D& aNormAxis, const QVector3D& aVec);

    static bool areSegmentsFacingEachOther(const util::Segment2D& aA, const util::Segment2D& aB)
    {
        if (QVector2D::dotProduct(aA.dir, aB.dir) < 0.0f)
            if (QVector2D::dotProduct(aB.start - aA.start, aA.dir) > 0.0f) return true;
        return  false;
    }

    static double remap(const double value, const double min, const double max, const double nMin, const double nMax)
    {
        return (((value - min) * (nMax - nMin)) / (max - min)) + nMin;
    }

    static double cycle( const double value, const double start, const double end )
    {
        const double length      = end - start;
        const double offsetValue = value - start; // value relative to 0
        return ( offsetValue - ( qFloor( offsetValue / length ) * length ) ) + start;
        // + start to reset back to start of original range
    }

private:
    MathUtil() {}
};

} // namespace util

#endif // UTIL_MATHUTIL_H
