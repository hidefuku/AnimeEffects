#ifndef IMG_RESOURCENODE_H
#define IMG_RESOURCENODE_H

#include <utility>
#include <QRect>
#include "util/TreeNodeBase.h"
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
    ResourceNode(const QString& aIdentifier);
    virtual ~ResourceNode();

    ResourceData& data() { return *mData; }
    const ResourceData& data() const { return *mData; }
    void resetData();
    void swapData(ResourceHandle& aHandle);

    ResourceHandle handle() const { ResourceHandle handle = mData; return std::move(handle); }

    bool isReferenced() const { return mData.use_count() > 1; }
    int getCountOfSameSiblings() const;

private:
    ResourceHandle mData;
};

} // namespace img

#endif // IMG_RESOURCENODE_H
