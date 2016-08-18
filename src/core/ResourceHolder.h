#ifndef CORE_RESOURCEHOLDER_H
#define CORE_RESOURCEHOLDER_H

#include <list>
#include <memory>
#include "util/NonCopyable.h"
#include "img/ResourceNode.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"

namespace core
{

class ResourceHolder
        : private util::NonCopyable
{
public:
    struct ImageTree
    {
        img::ResourceNode* topNode;
        QString filePath;
    };

    ResourceHolder();
    ~ResourceHolder();

    void setRootPath(const QString& aPath);

    void pushImageTree(
            img::ResourceNode& aTopNode,
            const QString& aAbsFilePath);

    ImageTree popImageTree();

    const std::list<ImageTree>& imageTrees() const { return mImageTrees; }

    QString changeFilePath(
            img::ResourceNode& aTopNode,
            const QString& aAbsFilePath);

    QString findFilePath(const img::ResourceNode& aTopNode) const;
    QString findRelativeFilePath(const img::ResourceNode& aTopNode) const;

    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    void destroy();
    bool serializeNode(Serializer& aOut, const img::ResourceNode& aNode) const;
    bool deserializeNode(Deserializer& aIn, img::ResourceNode** aDst);

    std::list<ImageTree> mImageTrees;
    QString mRootPath;
};

} // namespace core

#endif // CORE_RESOURCEHOLDER_H
