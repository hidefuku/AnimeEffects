#include "img/ResourceNode.h"
#include "img/BlendMode.h"
#include "core/ImageKey.h"
#include "core/Constant.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
ImageKey::Data::Data()
    : mEasing()
    , mResHandle()
    , mBlendMode(img::BlendMode_Normal)
    , mImageOffset()
    , mGridMesh()
{
}

ImageKey::Data::Data(const Data& aRhs)
    : mEasing(aRhs.mEasing)
    , mResHandle(aRhs.mResHandle)
    , mBlendMode(aRhs.mBlendMode)
    , mImageOffset()
{
    setImageOffset(aRhs.mImageOffset);
    mGridMesh = aRhs.mGridMesh;
}

ImageKey::Data& ImageKey::Data::operator=(const Data& aRhs)
{
    mEasing = aRhs.mEasing;
    mResHandle = aRhs.mResHandle;
    mBlendMode = aRhs.mBlendMode;
    setImageOffset(aRhs.mImageOffset);
    mGridMesh = aRhs.mGridMesh;
    return *this;
}

void ImageKey::Data::setImageOffset(const QVector2D& aOffset)
{
    const QVector2D offset(
                xc_clamp(aOffset.x(), Constant::transMin(), Constant::transMax()),
                xc_clamp(aOffset.y(), Constant::transMin(), Constant::transMax()));
    mImageOffset = offset;
    mGridMesh.setOriginOffset(offset);
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
    , mSleepCount(0)
{
    mData.resource().setOriginKeeping(true);
}

TimeKey* ImageKey::createClone()
{
    auto newKey = new ImageKey();
    newKey->mData = this->mData;
    newKey->resetTextureCache();
    return newKey;
}

void ImageKey::setImage(const img::ResourceHandle& aResource, img::BlendMode aMode)
{
    mData.setBlendMode(aMode);
    setImage(aResource);
}

void ImageKey::setImage(const img::ResourceHandle& aResource)
{
    mData.resource() = aResource;
    mData.setImageOffset(QVector2D());
    if (hasImage())
    {
        auto size = mData.resource()->image().pixelSize();
        mData.setImageOffset(QVector2D(-size.width() * 0.5f, -size.height() * 0.5f));
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

void ImageKey::sleep()
{
    ++mSleepCount;
    if (mSleepCount == 1)
    {
        mData.resource().setOriginKeeping(false);
    }
}

void ImageKey::awake()
{
    XC_ASSERT(mSleepCount > 0);
    --mSleepCount;
    if (mSleepCount == 0)
    {
        mData.resource().setOriginKeeping(true);
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
    // image offset
    aOut.write(mData.imageOffset());
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
    // image offset
    {
        QVector2D offset;
        aIn.read(offset);
        mData.setImageOffset(offset);
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

