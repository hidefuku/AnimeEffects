#include "img/ResourceNode.h"

namespace img
{

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

} // namespace img
