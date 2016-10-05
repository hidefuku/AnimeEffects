#include "ctrl/Painter.h"

namespace ctrl
{

#if USE_COREPROFILE_PAINTER
//-------------------------------------------------------------------------------------------------
GLCorePaintEngine::GLCorePaintEngine()
    : QPaintEngine(QPaintEngine::AlphaBlend |
                   QPaintEngine::Antialiasing |
                   QPaintEngine::ConstantOpacity |
                   QPaintEngine::PrimitiveTransform)
    , mDrawer()
{
}

bool GLCorePaintEngine::begin(QPaintDevice* aDevPtr)
{
    (void)aDevPtr;
    mDrawer.begin();
    return true;
}

bool GLCorePaintEngine::end()
{
    mDrawer.end();
    return true;
}

void GLCorePaintEngine::updateState(const QPaintEngineState& aState)
{
    if (aState.state() & QPaintEngine::DirtyBrush)
    {
        mDrawer.setBrush(aState.brush().color());
    }
    else if (aState.state() & QPaintEngine::DirtyPen)
    {
        auto pen = aState.pen();
        gl::PrimitiveDrawer::PenStyle style;
        switch (pen.style())
        {
        case Qt::NoPen:
            style = gl::PrimitiveDrawer::PenStyle_None; break;
        case Qt::SolidLine:
            style = gl::PrimitiveDrawer::PenStyle_Solid; break;
        case Qt::DotLine:
            style = gl::PrimitiveDrawer::PenStyle_Dot; break;
        case Qt::DashLine:
        case Qt::DashDotLine:
        case Qt::DashDotDotLine:
        case Qt::CustomDashLine:
            style = gl::PrimitiveDrawer::PenStyle_Dash; break;
        default:
            style = gl::PrimitiveDrawer::PenStyle_Solid; break;
        }
        mDrawer.setPen(pen.color(), pen.widthF(), style);
    }
    else if (aState.state() & QPaintEngine::DirtyHints)
    {
        const bool useAA = aState.renderHints() & QPainter::Antialiasing;
        mDrawer.setAntiAliasing(useAA);
    }
}

void GLCorePaintEngine::drawPoints(const QPoint* aPoints, int aCount)
{
    for (int i = 0; i < aCount; ++i) mDrawer.drawPoint(QPointF(aPoints[i]));
}

void GLCorePaintEngine::drawPoints(const QPointF* aPoints, int aCount)
{
    for (int i = 0; i < aCount; ++i) mDrawer.drawPoint(aPoints[i]);
}

void GLCorePaintEngine::drawLines(const QLine* aLines, int aCount)
{
    for (int i = 0; i < aCount; ++i) mDrawer.drawLine(QLineF(aLines[i]));
}

void GLCorePaintEngine::drawLines(const QLineF* aLines, int aCount)
{
    for (int i = 0; i < aCount; ++i) mDrawer.drawLine(aLines[i]);
}

void GLCorePaintEngine::drawRects(const QRect* aRects, int aCount)
{
    for (int i = 0; i < aCount; ++i) mDrawer.drawRect(QRectF(aRects[i]));
}

void GLCorePaintEngine::drawRects(const QRectF* aRects, int aCount)
{
    for (int i = 0; i < aCount; ++i) mDrawer.drawRect(aRects[i]);
}

void GLCorePaintEngine::drawEllipse(const QRectF& aRect)
{
    mDrawer.drawEllipse(aRect.center(), 0.5f * aRect.width(), 0.5f * aRect.height());
}

void GLCorePaintEngine::drawPath(const QPainterPath& aPath)
{
    (void)aPath;
}

void GLCorePaintEngine::drawPolygon(const QPoint* aPoints, int aCount, PolygonDrawMode aMode)
{
    (void)aMode;
    mDrawer.drawPolygon(aPoints, aCount);
}

void GLCorePaintEngine::drawPolygon(const QPointF* aPoints, int aCount, PolygonDrawMode aMode)
{
    (void)aMode;
    mDrawer.drawPolygon(aPoints, aCount);
}

void GLCorePaintEngine::drawImage(
        const QRectF& aRect, const QImage& aImage, const QRectF& aSrcRect, Qt::ImageConversionFlags aFlags)
{
    (void)aRect;
    (void)aImage;
    (void)aSrcRect;
    (void)aFlags;
}

void GLCorePaintEngine::drawPixmap(const QRectF& aRect, const QPixmap& aPixmap, const QRectF& aSrcRect)
{
    (void)aRect;
    (void)aPixmap;
    (void)aSrcRect;
}

void GLCorePaintEngine::drawTextItem(const QPointF& aPos, const QTextItem& aTextItem)
{
    (void)aPos;
    (void)aTextItem;
}

//-------------------------------------------------------------------------------------------------
int GLCorePaintDevice::metric(PaintDeviceMetric aMetric) const
{
    switch (aMetric)
    {
    case QPaintDevice::PdmWidth: return mOrigin.width();
    case QPaintDevice::PdmHeight: return mOrigin.height();
    case QPaintDevice::PdmWidthMM: return mOrigin.widthMM();
    case QPaintDevice::PdmHeightMM: return mOrigin.heightMM();
    case QPaintDevice::PdmNumColors: return mOrigin.colorCount();
    case QPaintDevice::PdmDepth: return mOrigin.depth();
    case QPaintDevice::PdmDpiX: return mOrigin.logicalDpiX();
    case QPaintDevice::PdmDpiY: return mOrigin.logicalDpiY();
    case QPaintDevice::PdmPhysicalDpiX: return mOrigin.physicalDpiX();
    case QPaintDevice::PdmPhysicalDpiY: return mOrigin.physicalDpiY();
    case QPaintDevice::PdmDevicePixelRatio: return mOrigin.devicePixelRatio();
    case QPaintDevice::PdmDevicePixelRatioScaled: return mOrigin.devicePixelRatioFScale();
    default: XC_ASSERT(0); return 0;
    }
}

//-------------------------------------------------------------------------------------------------
Painter::Painter()
{

}
#endif // USE_COREPROFILE_PAINTER

} // namespace ctrl
