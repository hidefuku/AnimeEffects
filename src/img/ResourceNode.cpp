#include "img/ResourceNode.h"

namespace img
{
#if 0
ResourceNode::ResourceNode(const QString& aIdentifier)
    : TreeNodeBase(this)
    , mBuffer()
    , mPos()
    , mUserData()
    , mIsLayer()
    , mIdentifier(aIdentifier)
    , mBlendMode(BlendMode_Normal)
    , mRefCount(0)
{
}

ResourceNode::~ResourceNode()
{
    XC_ASSERT(mRefCount == 0);
    qDeleteAll(children());
}

void ResourceNode::grabImage(const XCMemBlock& aBlock, const QSize& aSize, Format aFormat)
{
    mBuffer.grab(aFormat, aBlock, aSize);
}

XCMemBlock ResourceNode::releaseImage()
{
    return mBuffer.release();
}

void ResourceNode::setPos(const QPoint& aPos)
{
    mPos = aPos;
}

void ResourceNode::setBlendMode(BlendMode aMode)
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

ResourceHandle ResourceNode::handle()
{
    return ResourceHandle(*this);
}

void ResourceNode::incrementRC()
{
    XC_ASSERT(mRefCount >= 0);
    ++mRefCount;
}

void ResourceNode::decrementRC()
{
    XC_ASSERT(mRefCount > 0);
    --mRefCount;
}

int ResourceNode::getCountOfSameSiblings() const
{
    int count = 0;
    auto parent = this->parent();

    if (parent)
    {
        for (auto child : parent->children())
        {
            if (child != this && child->identifier() == this->identifier())
            {
                if (child->isLayer() == this->isLayer())
                {
                    ++count;
                }
            }
        }
    }
    return count;
}
#else

ResourceNode::ResourceNode(const QString& aIdentifier)
    : TreeNodeBase(this)
    , mData()
{
    mData = std::make_shared<ResourceData>(aIdentifier, this);
}

ResourceNode::~ResourceNode()
{
    qDeleteAll(children());
}

void ResourceNode::resetData()
{
    auto id = mData->identifier();
    mData = std::make_shared<ResourceData>(id, this);
}

void ResourceNode::swapData(ResourceHandle& aHandle)
{
    XC_ASSERT(aHandle->identifier() == mData->identifier());
    XC_ASSERT(aHandle->serialAddress() == mData->serialAddress());
    mData.swap(aHandle);
}

int ResourceNode::getCountOfSameSiblings() const
{
    int count = 0;
    auto parent = this->parent();

    if (parent)
    {
        for (auto child : parent->children())
        {
            if (child != this && child->mData->identifier() == this->mData->identifier())
            {
                if (child->mData->isLayer() == this->mData->isLayer())
                {
                    ++count;
                }
            }
        }
    }
    return count;
}
#endif

} // namespace img
