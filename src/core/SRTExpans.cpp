#include "util/MathUtil.h"
#include "core/SRTExpans.h"

namespace core
{
SRTExpans::SRTExpans()
    : mPos()
    , mRotate()
    , mScale(1.0f, 1.0f)
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

SRTKey::Data SRTExpans::data() const
{
    SRTKey::Data data;
    data.pos = mPos;
    data.rotate = mRotate;
    data.scale = mScale;
    return data;
}

void SRTExpans::setData(const SRTKey::Data& aData)
{
    mPos = aData.pos.toVector2D();
    mRotate = aData.rotate;
    mScale = aData.scale;
}

QMatrix4x4 SRTExpans::localMatrix() const
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

} // namespace core
