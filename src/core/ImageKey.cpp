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
    mCache.reset();
    mCache.reset(new Cache());

    if (mData.resource().hasImage())
    {
        auto imageData = mData.resource().image().data();
        auto pixelSize = mData.resource().image().pixelSize();

        // make a gl texture
        mCache->texture().create(pixelSize, imageData);
        mCache->texture().setFilter(GL_LINEAR);
        mCache->texture().setWrap(GL_CLAMP_TO_BORDER, QColor(0, 0, 0, 0));

        // grid mesh
        const int cellPx = std::max(std::min(8, pixelSize.width() / 4), 2);
        mCache->gridMesh().createFromImage(imageData, pixelSize, cellPx);
    }
}

bool ImageKey::serialize(Serializer& aOut) const
{
    // easing
    aOut.write(mData.easing());
    // image id
    aOut.writeID(mData.resource().serialAddress());

    return aOut.checkStream();
}

bool ImageKey::deserialize(Deserializer& aIn)
{
    mData.resource() = img::ResourceHandle();

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
            this->resetCache();
        };
        if (!aIn.orderIDData(solver))
        {
            return aIn.errored("invalid image reference id");
        }
    }

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core

