#include "img/ResourceHandle.h"
#include "img/ResourceNode.h"

namespace img
{

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

} // namespace img
