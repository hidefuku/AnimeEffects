#include <float.h>
#include "MathUtil.h"

namespace util
{

QVector2D MathUtil::getRotateVectorRad(const QVector2D& aVec, float aRotate)
{
    const float s = qSin(aRotate);
    const float c = qCos(aRotate);
    return QVector2D(c * aVec.x() - s * aVec.y(), s * aVec.x() + c * aVec.y());
}

QPointF MathUtil::getRotateVectorRad(const QPointF& aVec, float aRotate)
{
    const float s = qSin(aRotate);
    const float c = qCos(aRotate);
    return QPointF(c * aVec.x() - s * aVec.y(), s * aVec.x() + c * aVec.y());
}

QVector2D MathUtil::getAxisInversed(const QVector2D& aNormAxis, const QVector2D& aVec)
{
    return 2.0f * (aNormAxis * QVector2D::dotProduct(aNormAxis, aVec)) - aVec;
}

QVector3D MathUtil::getAxisInversed(const QVector3D& aNormAxis, const QVector3D& aVec)
{
    return 2.0f * (aNormAxis * QVector3D::dotProduct(aNormAxis, aVec)) - aVec;
}

float MathUtil::getClockwiseRotationRad(const QVector2D& aFrom, const QVector2D& aTo)
{
    if (aFrom.isNull() || aTo.isNull()) return 0.0f;

    const float from = getAngleRad(aFrom);
    const float to = getAngleRad(aTo);
    const float rotate = to - from;
    return rotate < 0 ? rotate : (float)(rotate - 2.0 * M_PI);
}

QVector2D MathUtil::blendVectorByClockwiseRotation(
        const QVector2D& aFrom, const QVector2D& aTo, float aRate)
{
    const float flen = aFrom.length();
    const float tlen = aTo.length();
    const float len = 0.5f * (flen + tlen);
    if (flen < FLT_EPSILON || tlen < FLT_EPSILON || len < FLT_EPSILON)
    {
        return aFrom;
    }
    const float rotate = getClockwiseRotationRad(aFrom, aTo);
    return getRotateVectorRad(aFrom, rotate * aRate).normalized() * len;
}

} // namespace util
