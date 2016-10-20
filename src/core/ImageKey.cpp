#include "img/ResourceNode.h"
#include "core/ImageKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
ImageKey::Data::Data()
    : mEasing()
    , mResHandle()
{
}

//-------------------------------------------------------------------------------------------------
ImageKey::Cache::Cache()
    : mTexture()
    , mGridMesh()
{
}

//-------------------------------------------------------------------------------------------------
ImageKey::ImageKey()
    : mData()
    , mCache()
{
}

void ImageKey::resetCache()
{
    resetGridMeshCache();
    resetTextureCache();
}

void ImageKey::resetGridMeshCache()
{
    if (mData.resource()->hasImage())
    {
        auto imageData = mData.resource()->image().data();
        auto pixelSize = mData.resource()->image().pixelSize();

        // grid mesh
        const int cellPx = std::max(std::min(8, pixelSize.width() / 4), 2);
        mCache.gridMesh().createFromImage(imageData, pixelSize, cellPx);
    }
}

void ImageKey::resetTextureCache()
{
    if (mData.resource()->hasImage())
    {
        auto imageData = mData.resource()->image().data();
        auto pixelSize = mData.resource()->image().pixelSize();

        // make a gl texture
        mCache.texture().create(pixelSize, imageData);
        mCache.texture().setFilter(GL_LINEAR);
        mCache.texture().setWrap(GL_CLAMP_TO_BORDER, QColor(0, 0, 0, 0));
    }
}

bool ImageKey::serialize(Serializer& aOut) const
{
    // easing
    aOut.write(mData.easing());
    // image id
    aOut.writeID(mData.resource()->serialAddress());
    // grid mesh
    if (!mCache.gridMesh().serialize(aOut))
    {
        return false;
    }

    return aOut.checkStream();
}

bool ImageKey::deserialize(Deserializer& aIn)
{
    mData.resource().reset();

    aIn.pushLogScope("ImageKey");

    // easing
    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }
    // image id
    {
        auto solver = [=](void* aPtr)
        {
            this->mData.resource() = ((img::ResourceNode*)aPtr)->handle();
            this->resetTextureCache();
        };
        if (!aIn.orderIDData(solver))
        {
            return aIn.errored("invalid image reference id");
        }
    }
    // grid mesh
    if (!mCache.gridMesh().deserialize(aIn))
    {
        return aIn.errored("failed to deserialize grid mesh");
    }

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core

