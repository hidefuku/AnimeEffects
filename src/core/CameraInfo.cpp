#include "XC.h"
#include "util/MathUtil.h"
#include "core/CameraInfo.h"

namespace
{
static const int kWheelValue = 120;
static const int kMinScaleIndex = -30 * kWheelValue;
static const int kMaxScaleIndex =  30 * kWheelValue;
}

namespace core
{

CameraInfo::CameraInfo()
    : mScreenSize()
    , mImageSize()
    , mPos()
    , mScale(1.0f)
    , mScaleIndex(0)
    , mRotate(0.0f)
{
}

void CameraInfo::setScale(float aScale)
{
    XC_ASSERT(aScale > 0.0f);
    mScale = aScale;
    mScaleIndex = (int)(mScale * kWheelValue);
}

void CameraInfo::setRotate(float aRadian)
{
    auto preCenter = mPos + toScreenVector(centerOffset());
    mRotate = aRadian;
    mPos = preCenter + toScreenVector(-centerOffset());
}

QVector2D CameraInfo::toScreenPos(const QVector2D& aWorldPos) const
{
    return mPos + toScreenVector(centerOffset()) +
            util::MathUtil::getRotateVectorRad(
                mScale * (aWorldPos - centerOffset()), mRotate);
}

QVector2D CameraInfo::toScreenVector(const QVector2D& aWorldVector) const
{
    return util::MathUtil::getRotateVectorRad(mScale * aWorldVector, mRotate);
}

QVector2D CameraInfo::toWorldPos(const QVector2D& aScreenPos) const
{
    return centerOffset() + util::MathUtil::getRotateVectorRad(
                aScreenPos - mPos - toScreenVector(centerOffset()), -mRotate) / mScale;
}

QVector2D CameraInfo::toWorldVector(const QVector2D& aScreenVector) const
{
    return util::MathUtil::getRotateVectorRad(aScreenVector, -mRotate) / mScale;
}

std::array<QVector2D, 4> CameraInfo::toScreenQuadangle(const QRectF& aWorldRect) const
{
    return std::array<QVector2D, 4>{
                toScreenPos(QVector2D(aWorldRect.topLeft())),
                toScreenPos(QVector2D(aWorldRect.bottomLeft())),
                toScreenPos(QVector2D(aWorldRect.bottomRight())),
                toScreenPos(QVector2D(aWorldRect.topRight()))};
}

QMatrix4x4 CameraInfo::viewMatrix() const
{
    static const float kNearPlane = -1000.0f;
    static const float kFarPlane = 1000.0f;

    QMatrix4x4 scr;
    scr.translate(mPos + toScreenVector(centerOffset()));
    scr.rotate(util::MathUtil::getDegreeFromRadian(mRotate), QVector3D(0.0f, 0.0f, 1.0f));
    scr.scale(mScale, mScale);
    scr.translate(-centerOffset());

    QMatrix4x4 view;
    view.ortho(0.0f, mScreenSize.width(),
               mScreenSize.height(), 0.0f,
               kNearPlane, kFarPlane);

    return view * scr;
}

void CameraInfo::updateByWheel(int aDelta, const QVector2D& aOriginPos)
{
    float preScale = std::max(mScale, 0.00001f);

    mScaleIndex -= aDelta;
    mScaleIndex = std::max(kMinScaleIndex, std::min(kMaxScaleIndex, mScaleIndex));

    if (mScaleIndex > 0)
    {
        mScale = (float)std::pow(1.1, (double)mScaleIndex / kWheelValue);
    }
    else if (mScaleIndex < 0)
    {
        mScale = (float)std::pow(0.9, -(double)mScaleIndex / kWheelValue);
    }
    else
    {
        mScale = 1.0f;
    }

    mPos = (mScale / preScale) * (mPos - aOriginPos) + aOriginPos;
    //XC_DEBUG_REPORT() << "wheel delta :" << aEvent->delta();
}

} // namespace core
