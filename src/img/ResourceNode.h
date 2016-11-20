#ifndef IMG_RESOURCENODE_H
#define IMG_RESOURCENODE_H

#include <utility>
#include <QRect>
#include "util/TreeNodeBase.h"
#include "util/TreeIterator.h"
#include "util/NonCopyable.h"
#include "img/Format.h"
#include "img/Buffer.h"
#include "img/ResourceHandle.h"
#include "img/BlendMode.h"

namespace img
{

class ResourceNode
        : public util::TreeNodeBase<ResourceNode>
        , private util::NonCopyable
{
public:
    typedef util::TreeNodeBase<ResourceNode>::Children ChildrenType;
    typedef util::TreeIterator<ResourceNode, ChildrenType::Iterator> Iterator;
    typedef util::TreeIterator<const ResourceNode, ChildrenType::ConstIterator> ConstIterator;

    ResourceNode(const QString& aIdentifier);
    virtual ~ResourceNode();

    ResourceData& data() { return *mHandle; }
    const ResourceData& data() const { return *mHandle; }
    void swapData(ResourceHandle& aRhs);
    ResourceHandle updateHandle(XCMemBlock aGrabbedImage, const QRect& aRect);

    ResourceHandle handle() const { return mHandle; }

    bool isReferenced() const { return mHandle.referenceCount() > 1; }
    bool isKeeped() const { return mHandle.originKeepingCount() > 0; }
    int getCountOfSameSiblings() const;

    void setAbandon(bool aIsAbandoned) { mIsAbandoned = aIsAbandoned; }
    bool isAbandoned() const { return mIsAbandoned; }

    QString treePath() const;

private:
    ResourceHandle mHandle;
    bool mIsAbandoned;
};

} // namespace img

#endif // IMG_RESOURCENODE_H
