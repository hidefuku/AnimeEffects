#include <utility>
#include <iostream>
#include <QFileInfo>
#include <QMessageBox>
#include "util/TextUtil.h"
#include "util/TreeUtil.h"
#include "img/PSDReader.h"
#include "img/PSDUtil.h"
#include "img/Util.h"
#include "img/BlendMode.h"
#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "gui/res/res_ImageSetter.h"
#include "gui/res/res_ResourceUpdater.h"

//#define RESOURCE_UPDATER_DUMP(...) XC_DEBUG_REPORT(__VA_ARGS__)
#define RESOURCE_UPDATER_DUMP(...)

namespace
{
//-------------------------------------------------------------------------------------------------
void pushTreeRowRecursive(
        util::TreePos& aDst, const QTreeWidget& aTree, QTreeWidgetItem* aObj)
{
    if (!aObj) return;

    QTreeWidgetItem* parent = aObj->parent();
    if (parent)
    {
        pushTreeRowRecursive(aDst, aTree, parent);
        const int index = parent->indexOfChild(aObj);
        XC_ASSERT(index >= 0);
        aDst.pushRow(index);
    }
    else
    {
        const int index = aTree.indexOfTopLevelItem(aObj);
        XC_ASSERT(index >= 0);
        aDst.pushRow(index);
    }
}

util::TreePos getTreePos(QTreeWidgetItem* aObj)
{
    util::TreePos pos;
    if (!aObj) return pos;

    const QTreeWidget* tree = aObj->treeWidget();
    if (!tree) return pos;

    pos.setValidity((bool)aObj);
    pushTreeRowRecursive(pos, *tree, aObj);
    return pos;
}

}

namespace gui {
namespace res {

//-------------------------------------------------------------------------------------------------
class NewTreePusher : public cmnd::Stable
{
    core::ResourceHolder& mHolder;
    img::ResourceNode* mTree;
    QString mAbsFilePath;
    bool mDone;
public:
    NewTreePusher(core::ResourceHolder& aHolder,
               img::ResourceNode* aTree,
               const QString& aFilePath)
        : mHolder(aHolder)
        , mTree(aTree)
        , mAbsFilePath(QFileInfo(aFilePath).absoluteFilePath())
        , mDone(false)
    {
        XC_PTR_ASSERT(aTree);
    }

    ~NewTreePusher()
    {
        if (!mDone)
        {
            delete mTree;
        }
    }

    virtual void undo()
    {
        mHolder.popImageTree();
        mDone = false;
    }

    virtual void redo()
    {
        mHolder.pushImageTree(*mTree, mAbsFilePath);
        mDone = true;
    }
};

//-------------------------------------------------------------------------------------------------
class TreeDeleter : public cmnd::Stable
{
    core::ResourceHolder& mHolder;
    core::ResourceHolder::ImageTree mTree;
    int mIndex;
    bool mDone;
public:
    TreeDeleter(core::ResourceHolder& aHolder, int aIndex)
        : mHolder(aHolder)
        , mTree()
        , mIndex(aIndex)
        , mDone(false)
    {
    }

    ~TreeDeleter()
    {
        if (mDone)
        {
            delete mTree.topNode;
        }
    }

    virtual void exec()
    {
        mTree = mHolder.imageTree(mIndex);
        XC_PTR_ASSERT(mTree.topNode);
        redo();
    }

    virtual void undo()
    {
        mHolder.insertImageTree(mTree, mIndex);
        mDone = false;
    }

    virtual void redo()
    {
        mHolder.removeImageTree(mIndex);
        mDone = true;
    }
};

//-------------------------------------------------------------------------------------------------
ResourceUpdater::ResourceUpdater(ViaPoint& aViaPoint, core::Project& aProject)
    : mViaPoint(aViaPoint)
    , mProject(aProject)
    , mPSDFormat()
{
}

//-------------------------------------------------------------------------------------------------
void ResourceUpdater::load(const QString& aFilePath)
{
    if (aFilePath.isEmpty()) return;

    auto newTree = createResourceTree(aFilePath, true);
    if (!newTree) return;

    {
        auto& stack = mProject.commandStack();
        auto& holder = mProject.resourceHolder();

        cmnd::ScopedMacro macro(stack, "add new resource");

        // notifier
        macro.grabListener(new AddNewOneNotifier(mViaPoint, mProject));

        stack.push(new NewTreePusher(holder, newTree, aFilePath));
    }
}

img::ResourceNode* ResourceUpdater::createResourceTree(const QString& aFilePath, bool aLoadImage)
{
    const QFileInfo fileInfo(aFilePath);
    if (!fileInfo.isFile()) return nullptr;

    if (fileInfo.suffix() == "psd")
    {
        return createPsdTree(aFilePath, aLoadImage);
    }
    else
    {
        return createQImageTree(aFilePath, aLoadImage);
    }

}

img::ResourceNode* ResourceUpdater::createQImageTree(const QString& aFilePath, bool aLoadImage) const
{
    const QFileInfo fileInfo(aFilePath);
    if (!fileInfo.isFile()) return nullptr;

    QImage image(aFilePath);
    if (image.isNull())
    {
        QMessageBox::warning(nullptr, "QImage Error", "Failed to load image file.");
        return nullptr;
    }
    return img::Util::createResourceNode(image, "topnode", aLoadImage);
}

img::ResourceNode* ResourceUpdater::createPsdTree(const QString& aFilePath, bool aLoadImage)
{
    // open file
    std::ifstream file(aFilePath.toLocal8Bit(), std::ios::binary);
    if (file.fail())
    {
        QMessageBox::warning(nullptr, "FileIO Error", "Can not found a PSD file.");
        return nullptr;
    }

    // read psd
    img::PSDReader reader(file);
    if (reader.resultCode() != img::PSDReader::ResultCode_Success)
    {
        const QString errorText =
                "error(" + QString::number(reader.resultCode()) + ") " +
                QString::fromStdString(reader.resultMessage());
        QMessageBox::warning(nullptr, "PSD Parse Error", errorText);
        return nullptr;
    }
    file.close();

    mPSDFormat = std::move(reader.format());

    // create resource tree
    return img::Util::createResourceNodes(*mPSDFormat, aLoadImage);
}

//-------------------------------------------------------------------------------------------------
void ResourceUpdater::reload(Item& aItem)
{
    auto& holder = mProject.resourceHolder();

    img::ResourceNode& node = aItem.node();
    img::ResourceNode& topNode = util::TreeUtil::getTreeRoot(node);
    QString filePath = holder.findFilePath(topNode);
    if (filePath.isEmpty()) return;

    QScopedPointer<img::ResourceNode> newTree(createResourceTree(filePath, false));
    if (!newTree) return;

    RESOURCE_UPDATER_DUMP("begin reload");

    // reload images
    if (!tryReloadCorrespondingImages(aItem, newTree.data()))
    {
        return;
    }
    RESOURCE_UPDATER_DUMP("end reload");
}

std::pair<int, img::ResourceNode*> findCorrespondingNode(
        const img::ResourceNode::Children& aSearchList,
        const img::ResourceNode& aNode)
{
    {
        const int count = aNode.getCountOfSameSiblings();
        if (count > 0) return std::pair<int, img::ResourceNode*>(count + 1, nullptr);
    }

    // search
    QVector<img::ResourceNode*> sameNames;
    const bool targetIsLayer = aNode.data().isLayer();
    for (auto searchChild : aSearchList)
    {
        if (searchChild->data().identifier() == aNode.data().identifier())
        {
            if (targetIsLayer == searchChild->data().isLayer())
            {
                sameNames.push_back(searchChild);
            }
        }
    }

    if (sameNames.count() == 1)
    {
        return std::pair<int, img::ResourceNode*>(1, sameNames.front());
    }
    else
    {
        return std::pair<int, img::ResourceNode*>(sameNames.count(), nullptr);
    }
}

std::pair<bool, QVector<img::ResourceNode*>> allChildrenCanBeIdentified(
        img::ResourceNode& aCurNode, img::ResourceNode& aNewNode)
{
    std::pair<bool, QVector<img::ResourceNode*>> result;
    result.first = true;

    for (auto child : aNewNode.children())
    {
        auto corresponds = findCorrespondingNode(aCurNode.children(), *child);
        if (corresponds.first > 1)
        {
            result.first = false;
            result.second.push_back(child);
        }
        else if (corresponds.first == 1)
        {
            auto childRes = allChildrenCanBeIdentified(*corresponds.second, *child);
            if (!childRes.first)
            {
                result.first = false;
                for (auto childError : childRes.second)
                {
                    result.second.push_back(childError);
                }
            }
        }
    }
    return result;
}

img::ResourceNode* createNewAppendNode(ModificationNotifier& aNotifier, img::ResourceNode& aNode)
{
    using img::PSDFormat;

    auto newNode = new img::ResourceNode(aNode.data().identifier());
    newNode->data().copyFrom(aNode.data());

    aNotifier.event().pushTarget(*newNode);

    if (newNode->data().isLayer())
    {
        auto success = newNode->data().loadImage();
        XC_ASSERT(success); (void)success;
        XC_ASSERT(newNode->data().hasImage());
    }

    for (auto child : aNode.children())
    {
        auto newChild = createNewAppendNode(aNotifier, *child);
        newNode->children().pushBack(newChild);
    }

    return newNode;
}

//-------------------------------------------------------------------------------------------------
void ResourceUpdater::createImageReloaderRecursive(
        cmnd::Stack& aStack, ModificationNotifier& aNotifier,
        img::ResourceNode& aCurNode, img::ResourceNode& aNewNode)
{
    using img::PSDFormat;

    RESOURCE_UPDATER_DUMP("reload image %s", aCurNode.data().identifier().toLatin1().data());

    // update abandon setting
    aCurNode.setAbandon(false);

    if (aCurNode.data().isLayer())
    {
        XC_ASSERT(aNewNode.data().isLayer());

        // load new image
        auto success = aNewNode.data().loadImage();
        XC_ASSERT(success); (void)success;

        // if layer data be modified
        if (!aCurNode.data().hasSameLayerDataWith(aNewNode.data()))
        {
            // push to targets
            aNotifier.event().pushTarget(aCurNode);

            // push reload image command
            XCMemBlock newImagePtr = aNewNode.data().image().block();
            const QRect newImageRect(aNewNode.data().pos(), aNewNode.data().image().pixelSize());
            aNewNode.data().releaseImage();
            aStack.push(new ImageSetter(aCurNode, newImagePtr, newImageRect));
        }
        else
        {
            aNewNode.data().freeImage();
        }
    }

    // each child
    for (auto child : aNewNode.children())
    {
        auto corresponds = findCorrespondingNode(aCurNode.children(), *child);
        XC_ASSERT(corresponds.first <= 1); // check identifiability

        if (corresponds.first == 1)
        {
            // reload node
            XC_PTR_ASSERT(corresponds.second);
            createImageReloaderRecursive(aStack, aNotifier, *corresponds.second, *child);
        }
        else
        {
            // append new node
            auto newChild = createNewAppendNode(aNotifier, *child);
            aStack.push(new cmnd::PushBackNewTreeObject<img::ResourceNode>(&aCurNode.children(), newChild));
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ResourceUpdater::createAbandonedImageRemoverRecursive(cmnd::Stack& aStack, img::ResourceNode& aNode)
{
    bool isKeeped = aNode.isKeeped();

    if (!isKeeped)
    {
        img::ResourceNode::ConstIterator itr(&aNode);
        while (itr.hasNext())
        {
            if (itr.next()->isKeeped())
            {
                isKeeped = true;
                break;
            }
        }
    }

    if (!isKeeped && aNode.isAbandoned())
    {
        XC_PTR_ASSERT(aNode.parent()); // topnode will never be abandoned
        aStack.push(new cmnd::RemoveTreeByObj<img::ResourceNode>(&aNode.parent()->children(), &aNode));
        aStack.push(new cmnd::GrabDeleteObject<img::ResourceNode>(&aNode));
        return;
    }

    for (auto child : aNode.children())
    {
        createAbandonedImageRemoverRecursive(aStack, *child);
    }
}

//-------------------------------------------------------------------------------------------------
bool ResourceUpdater::tryReloadCorrespondingImages(
        QTreeWidgetItem& aTarget, img::ResourceNode* aNewTree)
{
    XC_PTR_ASSERT(aNewTree);

    auto item = res::Item::cast(&aTarget);
    XC_PTR_ASSERT(item);
    img::ResourceNode& targetNode = item->node();

    // read current tree position
    std::list<img::ResourceNode*> pos;
    for (auto ptr = &targetNode; ptr->parent(); ptr = ptr->parent())
    {
        pos.push_front(ptr);
    }

    // find corresponding node
    img::ResourceNode* newNode = aNewTree;
    for (auto node : pos)
    {
        auto corresponds = findCorrespondingNode(newNode->children(), *node);
        if (corresponds.first != 1)
        {
            auto text = QString("Failed to find a corresponding node.") + " (" + node->data().identifier() + ")";
            QMessageBox::warning(nullptr, "Corresponding Error", text);
            return false;
        }
        XC_PTR_ASSERT(corresponds.second);
        newNode = corresponds.second;
    }

    // check reloadable
    auto beIdentified = allChildrenCanBeIdentified(targetNode, *newNode);
    if (!beIdentified.first)
    {
        auto text = QString("Failed to identify nodes by following duplications.\n");
        for (auto& duplicated : beIdentified.second)
        {
            text += duplicated->treePath() + "\n";
        }
        QMessageBox::warning(nullptr, "Corresponding Error", text);
        return false;
    }

    // reset abandon settings
    {
        img::ResourceNode::Iterator itr(&targetNode);
        while (itr.hasNext())
        {
            auto node = itr.next();
            if (!node->parent()) continue; // topnode never be abandoned
            node->setAbandon(true);
        }
    }

    // reload
    {
        RESOURCE_UPDATER_DUMP("create reload command %s, %s",
                              targetNode.data().identifier().toLatin1().data(),
                              newNode->data().identifier().toLatin1().data());

        auto& stack = mProject.commandStack();
        cmnd::ScopedMacro macro(stack, "reload images");

        // notifier
        auto notifier = new ModificationNotifier(mViaPoint, mProject, getTreePos(item));
        notifier->event().setRoot(targetNode);
        macro.grabListener(notifier);

        // create reload commands
        createImageReloaderRecursive(stack, *notifier, targetNode, *newNode);

        // create remove abandoned commands
        createAbandonedImageRemoverRecursive(stack, targetNode);

        // create key updating commands
        stack.push(mProject.objectTree().createResourceUpdater(notifier->event()));
    }

    return true;
}

} // namespace res
} // namespace gui
