#include "img/ResourceHandle.h"
#include "img/ResourceNode.h"

namespace img
{
#if 0
//-------------------------------------------------------------------------------------------------
ResourceHandle::ResourceHandle()
    : mNode()
{
}

ResourceHandle::ResourceHandle(ResourceNode& aNode)
    : mNode(&aNode)
{
    inc();
}

ResourceHandle::ResourceHandle(const ResourceHandle& aRhs)
    : mNode(aRhs.mNode)
{
    inc();
}

ResourceHandle::~ResourceHandle()
{
    dec();
}

ResourceHandle& ResourceHandle::operator =(const ResourceHandle& aRhs)
{
    dec();
    mNode = aRhs.mNode;
    inc();
    return *this;
}

bool ResourceHandle::hasImage() const
{
    XC_PTR_ASSERT(mNode);
    return mNode->hasImage();
}

const QString& ResourceHandle::identifier() const
{
    XC_PTR_ASSERT(mNode);
    return mNode->identifier();
}

const img::Buffer& ResourceHandle::image() const
{
    XC_PTR_ASSERT(mNode);
    return mNode->image();
}

QPoint ResourceHandle::pos() const
{
    XC_PTR_ASSERT(mNode);
    return mNode->pos();
}

BlendMode ResourceHandle::blendMode() const
{
    XC_PTR_ASSERT(mNode);
    return mNode->blendMode();
}

void ResourceHandle::inc()
{
    if (mNode)
    {
        mNode->incrementRC();
    }
}

void ResourceHandle::dec()
{
    if (mNode)
    {
        mNode->decrementRC();
    }
}
#else

ResourceData::ResourceData(const QString& aIdentifier, const void* aSerialAddress)
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

#endif
} // namespace img
