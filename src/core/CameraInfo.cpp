#include "XC.h"
#include "util/MathUtil.h"
#include "core/CameraInfo.h"

namespace core
{

CameraInfo::CameraInfo()
    : mScreenSize()
    , mImageSize()
    , mCenter()
    , mLeftTopPos()
    , mScale(1.0f)
    , mRotate(0.0f)
{
}

void CameraInfo::setLeftTopPos(const QVector2D& aPos)
{
    mLeftTopPos = aPos;
    mCenter = aPos + toScreenVector(centerOffset());
}

void CameraInfo::setCenter(const QVector2D& aCenter)
{
    mCenter = aCenter;
    mLeftTopPos = aCenter - toScreenVector(centerOffset());
}

QVector2D CameraInfo::leftTopPos() const
{
    return mLeftTopPos;
}

void CameraInfo::setScale(float aScale)
{
    XC_ASSERT(aScale > 0.0f);
    mScale = aScale;
    //mScaleIndex = (int)(mScale * kWheelValue);
    mLeftTopPos = mCenter - toScreenVector(centerOffset());
}

void CameraInfo::setRotate(float aRadian)
{
    mRotate = aRadian;
    mLeftTopPos = mCenter - toScreenVector(centerOffset());
}

QVector2D CameraInfo::screenCenter() const
{
    return QVector2D(0.5f * screenWidth(), 0.5f * screenHeight());
}

QVector2D CameraInfo::toScreenPos(const QVector2D& aWorldPos) const
{
    return mCenter + util::MathUtil::getRotateVectorRad(
                mScale * (aWorldPos - centerOffset()), mRotate);
}

QPointF CameraInfo::toScreenPos(const QPointF& aWorldPos) const
{
    return toScreenPos(QVector2D(aWorldPos)).toPointF();
}

QVector3D CameraInfo::toScreenPos(const QVector3D& aWorldPos) const
{
    return QVector3D(toScreenPos(aWorldPos.toVector2D()), aWorldPos.z());
}

QVector2D CameraInfo::toScreenVector(const QVector2D& aWorldVector) const
{
    return util::MathUtil::getRotateVectorRad(mScale * aWorldVector, mRotate);
}

QVector3D CameraInfo::toScreenVector(const QVector3D& aWorldVector) const
{
    return QVector3D(toScreenVector(aWorldVector.toVector2D()), aWorldVector.z());
}

float CameraInfo::toScreenLength(float aValue) const
{
    return aValue * mScale;
}

QVector2D CameraInfo::toWorldPos(const QVector2D& aScreenPos) const
{
    return centerOffset() + util::MathUtil::getRotateVectorRad(
                aScreenPos - mCenter, -mRotate) / mScale;
}

QVector3D CameraInfo::toWorldPos(const QVector3D& aScreenPos) const
{
    return QVector3D(toWorldPos(aScreenPos.toVector2D()), aScreenPos.z());
}

QPointF CameraInfo::toWorldPos(const QPointF& aScreenPos) const
{
    return toWorldPos(QVector2D(aScreenPos)).toPointF();
}

QVector2D CameraInfo::toWorldVector(const QVector2D& aScreenVector) const
{
    return util::MathUtil::getRotateVectorRad(aScreenVector, -mRotate) / mScale;
}

QVector3D CameraInfo::toWorldVector(const QVector3D& aScreenVector) const
{
    return QVector3D(toWorldVector(aScreenVector.toVector2D()), aScreenVector.z());
}

std::array<QVector2D, 4> CameraInfo::toScreenQuadangle(const QRectF& aWorldRect) const
{
    return std::array<QVector2D, 4>{
                toScreenPos(QVector2D(aWorldRect.topLeft())),
                toScreenPos(QVector2D(aWorldRect.bottomLeft())),
                toScreenPos(QVector2D(aWorldRect.bottomRight())),
                toScreenPos(QVector2D(aWorldRect.topRight()))};
}

std::array<QVector2D, 4> CameraInfo::toScreenQuadangle(const QRect& aWorldRect) const
{
    return toScreenQuadangle(QRectF(aWorldRect));
}

std::array<QVector2D, 4> CameraInfo::screenImageQuadangle() const
{
    return toScreenQuadangle(QRect(QPoint(0, 0), mImageSize));
}

QMatrix4x4 CameraInfo::viewMatrix() const
{
    static const float kNearPlane = -1000.0f;
    static const float kFarPlane = 1000.0f;

    QMatrix4x4 scr;
    scr.translate(mCenter);
    scr.rotate(util::MathUtil::getDegreeFromRadian(mRotate), QVector3D(0.0f, 0.0f, 1.0f));
    scr.scale(mScale, mScale);
    scr.translate(-centerOffset());

    QMatrix4x4 view;
    view.ortho(0.0f, mScreenSize.width(),
               mScreenSize.height(), 0.0f,
               kNearPlane, kFarPlane);

    return view * scr;
}

} // namespace core
