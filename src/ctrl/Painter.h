#ifndef CTRL_PAINTER_H
#define CTRL_PAINTER_H

#include <QPainter>
#include <QPaintEngine>
#include <QPaintDevice>
#include "gl/PrimitiveDrawer.h"

#define USE_COREPROFILE_PAINTER 1

namespace ctrl
{

#if USE_COREPROFILE_PAINTER

//-------------------------------------------------------------------------------------------------
class GLCorePaintEngine : public QPaintEngine
{
public:
    GLCorePaintEngine();
    virtual ~GLCorePaintEngine() {}

    void setViewMatrix(const QMatrix4x4& aMtx) { mDrawer.setViewMatrix(aMtx); }

    virtual QPaintEngine::Type type() const { return QPaintEngine::OpenGL2; }

    virtual bool begin(QPaintDevice* aDevPtr);
    virtual bool end();

    virtual void updateState(const QPaintEngineState& aState);

    virtual void drawPoints(const QPoint* aPoints, int aCount);
    virtual void drawPoints(const QPointF* aPoints, int aCount);

    virtual void drawLines(const QLine* aLines, int aCount);
    virtual void drawLines(const QLineF* aLines, int aCount);

    virtual void drawRects(const QRect* aRects, int aCount);
    virtual void drawRects(const QRectF* aRects, int aCount);

    virtual void drawEllipse(const QRectF& aRect);

    virtual void drawPath(const QPainterPath& aPath);

    virtual void drawPolygon(const QPoint* aPoints, int aCount, PolygonDrawMode aMode);
    virtual void drawPolygon(const QPointF* aPoints, int aCount, PolygonDrawMode aMode);

    virtual void drawImage(
            const QRectF& aRect, const QImage& aImage,
            const QRectF& aSrcRect, Qt::ImageConversionFlags aFlags = Qt::AutoColor);

    virtual void drawPixmap(const QRectF& aRect, const QPixmap& aPixmap, const QRectF& aSrcRect);

    //virtual void drawTiledPixmap(const QRectF& aRect, const QPixmap& aPixmap, const QPointF& aPos);

    virtual void drawTextItem(const QPointF& aPos, const QTextItem& aTextItem);

private:
    gl::PrimitiveDrawer mDrawer;
};

//-------------------------------------------------------------------------------------------------
class GLCorePaintDevice : public QPaintDevice
{
public:
    GLCorePaintDevice(const QPaintDevice& aOriginDevice, GLCorePaintEngine& aEngine)
        : mOrigin(aOriginDevice), mEngine(aEngine) {}
    virtual ~GLCorePaintDevice() {}
    virtual QPaintEngine* paintEngine() const { return &mEngine; }
    virtual int metric(PaintDeviceMetric aMetric) const;

private:
    const QPaintDevice& mOrigin;
    GLCorePaintEngine& mEngine;
};

//-------------------------------------------------------------------------------------------------
class Painter
{
public:
    Painter();

private:
};

#else // USE_COREPROFILE_PAINTER
typedef QPainter Painter;
#endif // USE_COREPROFILE_PAINTER

} // namespace ctrl

#endif // CTRL_PAINTER_H
