#include <QDir>
#include "img/BlendMode.h"
#include "core/ResourceHolder.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
ResourceHolder::ResourceHolder()
    : mImageTrees()
    , mRootPath()
{
    mRootPath = QDir::currentPath();
}

ResourceHolder::~ResourceHolder()
{
    destroy();
}

void ResourceHolder::setRootPath(const QString& aRootPath)
{
    QDir oldDir(mRootPath);
    QDir newDir(aRootPath);
    mRootPath = aRootPath;

    for (auto& data : mImageTrees)
    {
        auto absFilePath = oldDir.absoluteFilePath(data.filePath);
        data.filePath = newDir.relativeFilePath(absFilePath);
    }
}

void ResourceHolder::pushImageTree(
        img::ResourceNode& aGrabNode,
        const QString& aFilePath)
{
    QDir dir(mRootPath);
    ImageTree data;
    data.topNode = &aGrabNode;
    data.filePath = dir.relativeFilePath(aFilePath);
    mImageTrees.push_back(data);
}

ResourceHolder::ImageTree ResourceHolder::popImageTree()
{
    ImageTree tree = mImageTrees.back();
    mImageTrees.pop_back();
    return tree;
}

void ResourceHolder::insertImageTree(const ImageTree& aTree, int aIndex)
{
    int index = 0;
    for (auto itr = mImageTrees.begin(); itr != mImageTrees.end(); ++itr)
    {
        if (index == aIndex)
        {
            mImageTrees.insert(itr, aTree);
            return;
        }
        ++index;
    }
}

void ResourceHolder::removeImageTree(int aIndex)
{
    int index = 0;
    for (auto itr = mImageTrees.begin(); itr != mImageTrees.end(); ++itr)
    {
        if (index == aIndex)
        {
            mImageTrees.erase(itr);
            return;
        }
        ++index;
    }
}

ResourceHolder::ImageTree ResourceHolder::imageTree(int aIndex) const
{
    int index = 0;
    for (auto itr = mImageTrees.begin(); itr != mImageTrees.end(); ++itr)
    {
        if (index == aIndex) return *itr;
        ++index;
    }
    return ImageTree();
}

QString ResourceHolder::changeFilePath(
        img::ResourceNode& aTopNode,
        const QString& aAbsFilePath)
{
    QDir dir(mRootPath);

    for (auto& data : mImageTrees)
    {
        if (data.topNode == &aTopNode)
        {
            data.filePath = dir.relativeFilePath(aAbsFilePath);
            return data.filePath;
        }
    }
    return QString();
}

QString ResourceHolder::findFilePath(const img::ResourceNode& aTopNode) const
{
    for (auto& data : mImageTrees)
    {
        if (data.topNode == &aTopNode)
        {
            return QDir(mRootPath).absoluteFilePath(data.filePath);
        }
    }
    return QString();
}

QString ResourceHolder::findRelativeFilePath(const img::ResourceNode& aTopNode) const
{
    for (auto& data : mImageTrees)
    {
        if (data.topNode == &aTopNode)
        {
            return data.filePath;
        }
    }
    return QString();
}

void ResourceHolder::destroy()
{
    for (auto data : mImageTrees)
    {
        delete data.topNode;
    }
    mImageTrees.clear();
}

bool ResourceHolder::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'R', 'e', 's', 'o', 'u', 'r', 'c', 'e' };

    // signature
    auto pos = aOut.beginBlock(kSignature);

    // top node count
    aOut.write((int)mImageTrees.size());

    for (auto data : mImageTrees)
    {
        // file path
        aOut.write(data.filePath);

        // write nodes
        if (!serializeNode(aOut, *data.topNode))
        {
            return false;
        }
    }

    // end block
    aOut.endBlock(pos);

    return aOut.checkStream();
}

bool ResourceHolder::serializeNode(Serializer& aOut, const img::ResourceNode& aNode) const
{
    static const std::array<uint8, 8> kSignature =
        { 'R', 'e', 's', 'N', 'o', 'd', 'e', '_' };

    // block begin
    auto pos = aOut.beginBlock(kSignature);

    // identifier
    aOut.write(aNode.data().identifier());

    // child count
    aOut.write((int)aNode.children().size());

    // reference id
    aOut.writeID(&aNode);

    // is layer
    aOut.write(aNode.data().isLayer());

    // rect
    aOut.write(QRect(aNode.data().pos(), aNode.data().image().pixelSize()));

    // blend mode
    aOut.writeFixedString(img::getQuadIdFromBlendMode(aNode.data().blendMode()), 4);

    // memory block(null image is also ok)
    aOut.writeImage(aNode.data().image().block(), aNode.data().image().pixelSize());

    // block end
    aOut.endBlock(pos);

    // check failure
    if (aOut.failure())
    {
        return false;
    }

    // iterate children
    for (auto child : aNode.children())
    {
        XC_PTR_ASSERT(child);

        if (!serializeNode(aOut, *child))
        {
            return false;
        }
    }

    return aOut.checkStream();
}

bool ResourceHolder::deserialize(Deserializer& aIn)
{
    destroy();

    // check block begin
    if (!aIn.beginBlock("Resource"))
        return aIn.errored("invalid signature of resources");

    // dive log scope
    aIn.pushLogScope("Resources");


    // top node count
    int topNodeCount = 0;
    aIn.read(topNodeCount);

    // each tree
    for (int i = 0; i < topNodeCount; ++i)
    {
        ImageTree data;
        // file path
        aIn.read(data.filePath);

        // deserialize node
        if (!deserializeNode(aIn, &data.topNode))
        {
            return false;
        }

        mImageTrees.push_back(data);

        // progress report
        aIn.reportCurrent();
    }

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of resources");

    // rise log scope
    aIn.popLogScope();

    return aIn.checkStream();
}

bool ResourceHolder::deserializeNode(Deserializer& aIn, img::ResourceNode** aDst)
{
    *aDst = nullptr;

    // check block begin
    if (!aIn.beginBlock("ResNode_"))
        return aIn.errored("invalid signature of resource node");

    // identifier
    QString identifier;
    aIn.read(identifier);

    // child count
    int childCount = 0;
    aIn.read(childCount);
    if (childCount < 0)
        return aIn.errored("invalid child count");

    // create instance
    QScopedPointer<img::ResourceNode> node(new img::ResourceNode(identifier));
    auto nodePtr = node.data();
    if (!node)
        return aIn.errored("failed to create resource node");

    // reference id
    if (!aIn.bindIDData(nodePtr))
    {
        return aIn.errored("failed to bind reference id");
    }

    // is layer
    bool isLayer = false;
    aIn.read(isLayer);
    nodePtr->data().setIsLayer(isLayer);

    // rect
    QRect rect;
    aIn.read(rect);
    nodePtr->data().setPos(rect.topLeft());

    // blend mode
    {
        QString blendName;
        aIn.readFixedString(blendName, 4);
        auto blendMode = img::getBlendModeFromQuadId(blendName);
        if (blendMode == img::BlendMode_TERM)
        {
            return aIn.errored("invalid image blending mode");
        }
        nodePtr->data().setBlendMode(blendMode);
    }

    // memory block
    XCMemBlock block;
    if (!aIn.readImage(block))
    {
        return aIn.errored("invalid image resource");
    }

    if (block.data)
    {
        nodePtr->data().grabImage(block, rect.size(), img::Format_RGBA8);
    }

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of resource node");

    // check failure
    if (aIn.failure())
        return aIn.errored("stream error");

    // iterate children
    for (int i = 0; i < childCount; ++i)
    {
        img::ResourceNode* child = nullptr;
        if (!deserializeNode(aIn, &child)) return false;

        XC_PTR_ASSERT(child);
        nodePtr->children().pushBack(child);
    }

    // end
    if (!aIn.checkStream())
    {
        return false;
    }

    // success
    *aDst = node.take();
    return true;
}

} // namespace core
