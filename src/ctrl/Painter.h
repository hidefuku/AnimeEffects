#ifndef CTRL_PAINTER_H
#define CTRL_PAINTER_H

#include <functional>
#include <QPainter>
#include <QPaintEngine>
#include <QPaintDevice>
#include <QOpenGLTexture>
#include "gl/PrimitiveDrawer.h"
#include "gl/FontDrawer.h"
#include "gl/TextObject.h"

#define USE_COREPROFILE_PAINTER 1

namespace ctrl
{

#if USE_COREPROFILE_PAINTER

template<typename tKey, typename tCacheObj>
class CacheHolder
{
public:
    struct Cache
    {
        Cache() : obj(), size(), key(), used(false) {}
        QScopedPointer<tCacheObj> obj;
        size_t size;
        tKey key;
        bool used;
    };

    CacheHolder()
        : mCaches()
        , mUsedCacheLog()
        , mStorageSize()
    {}

    ~CacheHolder()
    {
        deleteAll();
    }

    tCacheObj* get(tKey aKey, const std::function<Cache*()>& aCreator)
    {
        Cache*& cache = mCaches[aKey];
        if (!cache)
        {
            cache = aCreator();
            cache->used = true;
            mUsedCacheLog.push_front(cache);
            //qDebug() << "create" << cache;
        }
        else
        {
            cache->used = true;
            mUsedCacheLog.removeOne(cache);
            mUsedCacheLog.push_front(cache);
            //qDebug() << "found" << cache;
        }
        return cache->obj.data();
    }

    // call it on begining of the rendering
    void reduceByStorageSize()
    {
        size_t currentSize = 0;
        for (auto itr = mUsedCacheLog.begin(); itr != mUsedCacheLog.end(); ++itr)
        {
            auto usedCache = *itr;
            if (currentSize + usedCache->size > mStorageSize)
            {
                // delete it and after
                while (itr != mUsedCacheLog.end())
                {
                    usedCache = *itr;
                    mCaches.remove(usedCache->key);
                    itr = mUsedCacheLog.erase(itr);
                    delete usedCache;
                    //qDebug() << "reduce" << usedCache;
                }
                break;
            }
        }
    }

    // call it on end of the rendering
    void updateStorageSize()
    {
        size_t usedSize = 0;
        for (auto& cache : mCaches)
        {
            if (cache->used)
            {
                usedSize += cache->size;
            }
            cache->used = false;
        }
        //qDebug() << "update size" << mStorageSize << usedSize;
        mStorageSize = std::max(mStorageSize, usedSize);
    }

    void deleteAll()
    {
        qDeleteAll(mCaches);
        mCaches.clear();
        mUsedCacheLog.clear();
    }

private:
    QMap<tKey, Cache*> mCaches;
    QList<Cache*> mUsedCacheLog;
    size_t mStorageSize;
};

//-------------------------------------------------------------------------------------------------
class GLCorePaintEngine : public QPaintEngine
{
public:
    GLCorePaintEngine();

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
    typedef CacheHolder<qint64, QOpenGLTexture> TextureCaches;
    typedef CacheHolder<gl::TextObject::MapKey, gl::TextObject> TextCaches;

    gl::PrimitiveDrawer mDrawer;
    gl::FontDrawer mFontDrawer;
    TextureCaches mTextureCaches;
    TextCaches mTextCaches;
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
