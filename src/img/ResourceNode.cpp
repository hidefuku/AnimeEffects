#include <memory>
#include "img/ResourceNode.h"

namespace img
{

ResourceNode::ResourceNode(const QString& aIdentifier)
    : TreeNodeBase(this)
    , mHandle(new ResourceData(aIdentifier, this), new int(0))
    , mIsAbandoned()
{
}

ResourceNode::~ResourceNode()
{
    qDeleteAll(children());
}

ResourceHandle ResourceNode::updateHandle(XCMemBlock aGrabbedImage, const QRect& aRect)
{
    ResourceHandle oldHandle = mHandle;
    auto id = mHandle->identifier();

    mHandle = ResourceHandle(new ResourceData(id, this), new int(0));
    *mHandle = *oldHandle;
    mHandle->setPos(aRect.topLeft());
    mHandle->grabImage(aGrabbedImage, aRect.size(), img::Format_RGBA8);

    return oldHandle;
}

void ResourceNode::swapData(ResourceHandle& aRhs)
{
    XC_ASSERT(aRhs->identifier() == mHandle->identifier());
    XC_ASSERT(aRhs->serialAddress() == mHandle->serialAddress());
    mHandle.swapData(aRhs);
}

int ResourceNode::getCountOfSameSiblings() const
{
    int count = 0;
    auto parent = this->parent();

    if (parent)
    {
        for (auto child : parent->children())
        {
            if (child != this && child->mHandle->identifier() == this->mHandle->identifier())
            {
                if (child->mHandle->isLayer() == this->mHandle->isLayer())
                {
                    ++count;
                }
            }
        }
    }
    return count;
}

QString ResourceNode::treePath() const
{
    QStringList path;
    for (auto p = this; p != nullptr; p = p->parent())
    {
        path.push_front(p->data().identifier());
    }
    return path.join("/");
}

} // namespace img
