#ifndef CORE_IMAGEKEY_H
#define CORE_IMAGEKEY_H

#include "util/Easing.h"
#include "gl/Texture.h"
#include "img/ResourceHandle.h"
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
    public:
        Data();
        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }
        img::ResourceHandle& resource() { return mResHandle; }
        const img::ResourceHandle& resource() const { return mResHandle; }
    };

    class Cache
    {
        gl::Texture mTexture;
        GridMesh mGridMesh;
    public:
        Cache();
        gl::Texture& texture() { return mTexture; }
        const gl::Texture& texture() const { return mTexture; }
        GridMesh& gridMesh() { return mGridMesh; }
        const GridMesh& gridMesh() const { return mGridMesh; }
    };

    ImageKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    void resetCache();
    bool hasCache() const { return mCache; }
    Cache& cache() { XC_ASSERT(mCache); return *mCache; }
    const Cache& cache() const { XC_ASSERT(mCache); return *mCache; }

    virtual TimeKeyType type() const { return TimeKeyType_Image; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
    QScopedPointer<Cache> mCache;
};

} // namespace core

#endif // CORE_IMAGEKEY_H
