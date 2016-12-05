#ifndef CORE_IMAGEKEY_H
#define CORE_IMAGEKEY_H

#include "util/Easing.h"
#include "gl/Texture.h"
#include "img/ResourceHandle.h"
#include "img/BlendMode.h"
#include "core/GridMesh.h"
#include "core/TimeKey.h"

namespace core
{

class ImageKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        img::ResourceHandle mResHandle;
        img::BlendMode mBlendMode;
        QVector2D mImageOffset;
        GridMesh mGridMesh;
    public:
        Data();
        Data(const Data& aRhs);
        Data& operator=(const Data& aRhs);
        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }
        img::ResourceHandle& resource() { return mResHandle; }
        const img::ResourceHandle& resource() const { return mResHandle; }
        void setBlendMode(img::BlendMode aMode) { mBlendMode = aMode; }
        img::BlendMode blendMode() const { return mBlendMode; }
        void setImageOffset(const QVector2D& aOffset);
        const QVector2D& imageOffset() const { return mImageOffset; }
        GridMesh& gridMesh() { return mGridMesh; }
        const GridMesh& gridMesh() const { return mGridMesh; }
    };

    class Cache
    {
        gl::Texture mTexture;
    public:
        Cache();
        gl::Texture& texture() { return mTexture; }
        const gl::Texture& texture() const { return mTexture; }
    };

    ImageKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    Cache& cache() { return mCache; }
    const Cache& cache() const { return mCache; }

    void setImage(const img::ResourceHandle& aResource, img::BlendMode aMode);
    void setImage(const img::ResourceHandle& aResource);
    bool hasImage() const { return mData.resource() && mData.resource()->hasImage(); }

    virtual TimeKeyType type() const { return TimeKeyType_Image; }
    virtual bool canHoldChild() const { return true; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

    virtual void sleep();
    virtual void awake();

private:
    void resetGridMesh();
    void resetTextureCache();

    Data mData;
    Cache mCache;
    int mSleepCount;
};

} // namespace core

#endif // CORE_IMAGEKEY_H
