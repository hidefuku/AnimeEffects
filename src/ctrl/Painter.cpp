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
    , mFontDrawer()
    , mTextureCaches()
    , mTextCaches()
{
}

bool GLCorePaintEngine::begin(QPaintDevice* aDevPtr)
{
    //qDebug() << "begin";
    (void)aDevPtr;
    mTextureCaches.reduceByStorageSize();
    mTextCaches.reduceByStorageSize();
    mDrawer.begin();
    return true;
}

bool GLCorePaintEngine::end()
{
    mDrawer.end();
    mTextureCaches.updateStorageSize();
    mTextCaches.updateStorageSize();
    return true;
}

void GLCorePaintEngine::updateState(const QPaintEngineState& aState)
{
    if (aState.state() & QPaintEngine::DirtyBrush)
    {
        mDrawer.setBrushEnable(aState.brush().style() != Qt::NoBrush);
        mDrawer.setBrush(aState.brush().color());
    }

    if (aState.state() & QPaintEngine::DirtyPen)
    {
        auto pen = aState.pen();
        bool hasPen = true;
        gl::PrimitiveDrawer::PenStyle style;

        switch (pen.style())
        {
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
            hasPen = false; // Qt::NoPen will come here.
            style = gl::PrimitiveDrawer::PenStyle_Solid;
            break;
        }
        mDrawer.setPenEnable(hasPen);
        mDrawer.setPen(pen.color(), pen.widthF(), style);
    }

    if (aState.state() & QPaintEngine::DirtyHints)
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
    (void)aFlags;
    const QImage* ptr = &aImage;
    const qint64 key = aImage.cacheKey();

    auto texture = mTextureCaches.get(key, [=]()
    {
        auto cache = new TextureCaches::Cache();
        cache->obj.reset(new QOpenGLTexture(ptr->mirrored()));
        cache->key = key;
        cache->size = (size_t)ptr->byteCount();

        cache->obj->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        cache->obj->setMagnificationFilter(QOpenGLTexture::Linear);

        return cache;
    });
    mDrawer.drawTexture(aRect, texture->textureId(), QSize(texture->width(), texture->height()), aSrcRect);
}

void GLCorePaintEngine::drawPixmap(const QRectF& aRect, const QPixmap& aPixmap, const QRectF& aSrcRect)
{
    if (aPixmap.paintEngine()->type() == QPaintEngine::Raster && !aPixmap.isQBitmap())
    {
        const QPixmap* ptr = &aPixmap;
        const qint64 key = aPixmap.cacheKey();

        auto texture = mTextureCaches.get(key, [=]()
        {
            auto cache = new TextureCaches::Cache();
            auto image = ptr->toImage();
            cache->obj.reset(new QOpenGLTexture(image.mirrored()));
            cache->key = key;
            cache->size = (size_t)image.byteCount();

            cache->obj->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
            cache->obj->setMagnificationFilter(QOpenGLTexture::Linear);

            return cache;
        });
        mDrawer.drawTexture(aRect, texture->textureId(), QSize(texture->width(), texture->height()), aSrcRect);
    }
}

void GLCorePaintEngine::drawTextItem(const QPointF& aPos, const QTextItem& aTextItem)
{
    //mDrawer.end();
    auto key = gl::TextObject::getMapKey(aTextItem.text());

    auto textObj = mTextCaches.get(key, [&]()
    {
        auto obj = new gl::TextObject(aTextItem.text());

        this->mFontDrawer.setColor(QColor(255, 255, 255, 255));
        this->mFontDrawer.draw(aTextItem.font(), *obj);

        auto cache = new TextCaches::Cache();
        cache->obj.reset(obj);
        cache->key = key;
        cache->size = (size_t)obj->pixelCount();
        return cache;
    });
    auto offs = QPointF(0.0f, -aTextItem.ascent());

    //mDrawer.begin();
    mDrawer.drawTexture(QRectF(aPos + offs, QSizeF(textObj->texture().size())), textObj->texture());
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
