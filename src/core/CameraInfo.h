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

    void reset(const QSize& aScreenSize, double aDpr,
               const QSize& aImageSize, const QPoint& aLeftTopPos);

    void setDevicePixelRatio(double aRatio);
    inline void setScreenWidth(int aWidth) { mScreenSize.setWidth(aWidth); }
    inline void setScreenHeight(int aHeight) { mScreenSize.setHeight(aHeight); }
    inline void setScreenSize(const QSize& aSize) { mScreenSize = aSize; }
    inline void setImageSize(const QSize& aSize) { mImageSize = aSize; }
    void setLeftTopPos(const QVector2D& aPos);
    void setCenter(const QVector2D& aPos);
    void setScale(float aScale);
    void setRotate(float aRadian);

    inline double devicePixelRatio() const { return mDevicePixelRatio; }
    inline int screenWidth() const { return mScreenSize.width(); }
    inline int screenHeight() const { return mScreenSize.height(); }
    inline QSize screenSize() const { return mScreenSize; }
    inline QSize deviceScreenSize() const { return mScreenSize * mDevicePixelRatio; }
    QVector2D screenCenter() const;

    inline QSize imageSize() const { return mImageSize; }
    inline QVector2D center() const { return mCenter; }
    inline float scale() const { return mScale; }
    inline float rotate() const { return mRotate; }
    QVector2D leftTopPos() const;

    QVector2D toScreenPos(const QVector2D& aWorldPos) const;
    QPointF toScreenPos(const QPointF& aWorldPos) const;
    QVector3D toScreenPos(const QVector3D& aWorldPos) const;

    QVector2D toScreenVector(const QVector2D& aWorldVector) const;
    QVector3D toScreenVector(const QVector3D& aWorldVector) const;

    float toScreenLength(float aValue) const;

    std::array<QVector2D, 4> toScreenQuadangle(const QRectF& aWorldRect) const;
    std::array<QVector2D, 4> toScreenQuadangle(const QRect& aWorldRect) const;
    std::array<QVector2D, 4> screenImageQuadangle() const;

    QVector2D toWorldPos(const QVector2D& aScreenPos) const;
    QVector3D toWorldPos(const QVector3D& aScreenPos) const;
    QPointF toWorldPos(const QPointF& aScreenPos) const;

    QVector2D toWorldVector(const QVector2D& aScreenVector) const;
    QVector3D toWorldVector(const QVector3D& aScreenVector) const;

    float toWorldLength(float aValue) const;

    QMatrix4x4 viewMatrix() const;

private:
    QVector2D centerOffset() const
    {
        return 0.5f * QVector2D(mImageSize.width(), mImageSize.height());
    }

    double mDevicePixelRatio;
    QSize mScreenSize;
    QSize mImageSize;
    QVector2D mCenter;
    QVector2D mLeftTopPos;
    float mScale;
    float mRotate;
};

} // namespace core

#endif // CORE_CAMERAINFO_H
