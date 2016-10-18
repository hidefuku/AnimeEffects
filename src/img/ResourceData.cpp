#include <algorithm>
#include "img/ResourceNode.h"
#include "img/ResourceHandle.h"

namespace img
{

ResourceData::ResourceData(const QString& aIdentifier, const ResourceNode* aSerialAddress)
    : mBuffer()
    , mPos()
    , mUserData()
    , mIsLayer()
    , mIdentifier(aIdentifier)
    , mBlendMode(BlendMode_Normal)
    , mSerialAddress(aSerialAddress)
{
}

void ResourceData::grabImage(const XCMemBlock& aBlock, const QSize& aSize, Format aFormat)
{
    mBuffer.grab(aFormat, aBlock, aSize);
}

XCMemBlock ResourceData::releaseImage()
{
    return mBuffer.release();
}

void ResourceData::freeImage()
{
    mBuffer.free();
}

void ResourceData::setPos(const QPoint& aPos)
{
    mPos = aPos;
}

void ResourceData::setBlendMode(BlendMode aMode)
{
    if (aMode != BlendMode_TERM)
    {
        mBlendMode = aMode;
    }
    else
    {
        mBlendMode = BlendMode_Normal;
    }
}

bool ResourceData::hasSameLayerDataWith(const ResourceData& aData)
{
    if (!isLayer() || !aData.isLayer()) return false;
    if (pos() != aData.pos()) return false;
    if (blendMode() != aData.blendMode()) return false;
    if (!hasImage()) return !aData.hasImage();
    if (!aData.hasImage()) return false;
    if (image().pixelSize() != aData.image().pixelSize()) return false;
    if (image().format() != aData.image().format()) return false;

    auto size = image().size();
    XC_ASSERT(size == aData.image().size());
    auto ptr1 = image().data();
    auto ptr2 = aData.image().data();
    return std::equal(ptr1, ptr1 + size, ptr2);
}

} // namespace img
