#include "util/MathUtil.h"
#include "core/SRTExpans.h"

namespace core
{
SRTExpans::SRTExpans()
    : mPos()
    , mRotate()
    , mScale(1.0f, 1.0f)
    , mCentroid()
    , mSpline()
    , mParentMatrix()
    , mSplineCache()
{
}

bool SRTExpans::hasSplineCache(Frame aFrame) const
{
    if (aFrame <= 0 || mSplineCache.min() <= 0) return false;

    if (mSplineCache.isNegative())
    {
        return aFrame >= mSplineCache.min() || aFrame <= mSplineCache.max();
    }
    else
    {
        return mSplineCache.contains(aFrame.getDecimal());
    }
}

QMatrix4x4 SRTExpans::localCSRTMatrix() const
{
    QMatrix4x4 mtx = localSRTMatrix();
    mtx.translate(mCentroid);
    return mtx;
}

QMatrix4x4 SRTExpans::localSRTMatrix() const
{
    QMatrix4x4 mtx;
    mtx.translate(QVector3D(mPos));
    mtx.rotate(util::MathUtil::getDegreeFromRadian(mRotate), QVector3D(0.0f, 0.0f, 1.0f));
    mtx.scale(QVector3D(mScale, 1.0f));
    return mtx;
}

QMatrix4x4 SRTExpans::localSRMatrix() const
{
    QMatrix4x4 mtx;
    mtx.rotate(util::MathUtil::getDegreeFromRadian(mRotate), QVector3D(0.0f, 0.0f, 1.0f));
    mtx.scale(QVector3D(mScale, 1.0f));
    return mtx;
}

QMatrix4x4 SRTExpans::getLocalSRMatrix(float aRotate, const QVector2D& aScale)
{
    QMatrix4x4 mtx;
    mtx.rotate(util::MathUtil::getDegreeFromRadian(aRotate), QVector3D(0.0f, 0.0f, 1.0f));
    mtx.scale(QVector3D(aScale, 1.0f));
    return mtx;
}

void SRTExpans::setParentMatrix(const QMatrix4x4& aWorldCSRTMtx)
{
    mParentMatrix = aWorldCSRTMtx;
}

const QMatrix4x4& SRTExpans::parentMatrix() const
{
    return mParentMatrix;
}

QMatrix4x4 SRTExpans::worldCSRTMatrix() const
{
    return mParentMatrix * localCSRTMatrix();
}

QMatrix4x4 SRTExpans::worldSRTMatrix() const
{
    return mParentMatrix * localSRTMatrix();
}

} // namespace core
