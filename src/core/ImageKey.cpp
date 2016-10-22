#include "img/ResourceNode.h"
#include "img/BlendMode.h"
#include "core/ImageKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
ImageKey::Data::Data()
    : mEasing()
    , mResHandle()
    , mBlendMode(img::BlendMode_Normal)
    , mCenterOffset()
    , mGridMesh()
{
}

//-------------------------------------------------------------------------------------------------
ImageKey::Cache::Cache()
    : mTexture()
{
}

//-------------------------------------------------------------------------------------------------
ImageKey::ImageKey()
    : mData()
    , mCache()
{
}

void ImageKey::setImage(const img::ResourceHandle& aResource, img::BlendMode aMode)
{
    mData.setBlendMode(aMode);
    setImage(aResource);
}

void ImageKey::setImage(const img::ResourceHandle& aResource)
{
    mData.resource() = aResource;
    mData.setCenterOffset(QVector2D());
    if (hasImage())
    {
        auto size = mData.resource()->image().pixelSize();
        mData.setCenterOffset(QVector2D(size.width() * 0.5f, size.height() * 0.5f));
    }
    resetGridMesh();
    resetTextureCache();
}

void ImageKey::resetGridMesh()
{
    if (mData.resource()->hasImage())
    {
        auto imageData = mData.resource()->image().data();
        auto pixelSize = mData.resource()->image().pixelSize();

        // grid mesh
        const int cellPx = std::max(std::min(8, pixelSize.width() / 4), 2);
        mData.gridMesh().createFromImage(imageData, pixelSize, cellPx);
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
    // blend mode
    aOut.writeFixedString(img::getQuadIdFromBlendMode(mData.blendMode()), 4);
    // center offset
    aOut.write(mData.centerOffset());
    // grid mesh
    if (!mData.gridMesh().serialize(aOut))
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
    // blend mode
    {
        QString bname;
        aIn.readFixedString(bname, 4);
        auto bmode = img::getBlendModeFromQuadId(bname);
        if (bmode == img::BlendMode_TERM)
        {
            return aIn.errored("invalid image blending mode");
        }
        mData.setBlendMode(bmode);
    }
    // center offset
    {
        QVector2D centerOffs;
        aIn.read(centerOffs);
        mData.setCenterOffset(centerOffs);
    }
    // grid mesh
    if (!mData.gridMesh().deserialize(aIn))
    {
        return aIn.errored("failed to deserialize grid mesh");
    }

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core

