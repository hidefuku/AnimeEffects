#include "img/ResourceNode.h"

namespace img
{

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

} // namespace img
