#ifndef CORE_CAMERAINFO_H
#define CORE_CAMERAINFO_H

#include <array>
#include <QVector2D>
#include <QVector3D>
#include <QSize>
#include <QRectF>
#include <QPointF>
#include <QMatrix4x4>

namespace core
{

class CameraInfo
{
public:
    CameraInfo();

    inline void setScreenWidth(int aWidth) { mScreenSize.setWidth(aWidth); }
    inline void setScreenHeight(int aHeight) { mScreenSize.setHeight(aHeight); }
    inline void setScreenSize(const QSize& aSize) { mScreenSize = aSize; }
    inline void setImageSize(const QSize& aSize) { mImageSize = aSize; }
    inline void setPos(const QVector2D& aPos) { mPos = aPos; }
    void setScale(float aScale);
    void setRotate(float aRadian);

    inline int screenWidth() const { return mScreenSize.width(); }
    inline int screenHeight() const { return mScreenSize.height(); }
    inline QSize screenSize() const { return mScreenSize; }
    inline QSize imageSize() const { return mImageSize; }
    inline QVector2D pos() const { return mPos; }
    inline float scale() const { return mScale; }

    inline std::array<QVector2D, 4> screenImageQuadangle() const
    {
        return toScreenQuadangle(QRect(QPoint(0, 0), mImageSize));
    }

    QVector2D toScreenPos(const QVector2D& aWorldPos) const;

    inline QPointF toScreenPos(const QPointF& aWorldPos) const
    {
        return toScreenPos(QVector2D(aWorldPos)).toPointF();
    }

    inline QVector3D toScreenPos(const QVector3D& aWorldPos) const
    {
        return QVector3D(toScreenPos(aWorldPos.toVector2D()), aWorldPos.z());
    }

    QVector2D toScreenVector(const QVector2D& aWorldVector) const;

    inline QVector3D toScreenVector(const QVector3D& aWorldVector) const
    {
        return QVector3D(toScreenVector(aWorldVector.toVector2D()), aWorldVector.z());
    }

    inline float toScreenLength(float aValue) const
    {
        return aValue * mScale;
    }

    std::array<QVector2D, 4> toScreenQuadangle(const QRectF& aWorldRect) const;

    inline std::array<QVector2D, 4> toScreenQuadangle(const QRect& aWorldRect) const
    {
        return toScreenQuadangle(QRectF(aWorldRect));
    }

    QVector2D toWorldPos(const QVector2D& aScreenPos) const;

    inline QVector3D toWorldPos(const QVector3D& aScreenPos) const
    {
        return QVector3D(toWorldPos(aScreenPos.toVector2D()), aScreenPos.z());
    }

    inline QPointF toWorldPos(const QPointF& aScreenPos) const
    {
        return toWorldPos(QVector2D(aScreenPos)).toPointF();
    }

    QVector2D toWorldVector(const QVector2D& aScreenVector) const;

    inline QVector3D toWorldVector(const QVector3D& aScreenVector) const
    {
        return QVector3D(toWorldVector(aScreenVector.toVector2D()), aScreenVector.z());
    }

    QMatrix4x4 viewMatrix() const;

    void updateByWheel(int aDelta, const QVector2D& aOriginPos);

private:
    QVector2D centerOffset() const
    {
        return 0.5f * QVector2D(mImageSize.width(), mImageSize.height());
    }
    QVector2D center() const { return mPos + centerOffset(); }

    QSize mScreenSize;
    QSize mImageSize;
    QVector2D mPos;
    float mScale;
    int mScaleIndex;
    float mRotate;
};

} // namespace core

#endif // CORE_CAMERAINFO_H
