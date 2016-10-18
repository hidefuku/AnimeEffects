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

} // namespace img
