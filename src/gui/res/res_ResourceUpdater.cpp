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
ResourceUpdater::ResourceUpdater(ViaPoint& aViaPoint, core::Project& aProject)
    : mViaPoint(aViaPoint)
    , mProject(aProject)
{
}

void ResourceUpdater::load(const QString& aFilePath)
{
    if (aFilePath.isEmpty()) return;

    // open file
    std::ifstream file(aFilePath.toLocal8Bit(), std::ios::binary);
    if (file.fail())
    {
        QMessageBox msgBox;
        msgBox.setText("can not found a resource file.");
        msgBox.exec();
        return;
    }

    // read psd
    img::PSDReader reader(file);
    if (reader.resultCode() != img::PSDReader::ResultCode_Success)
    {
        const QString errorText =
                "error(" + QString::number(reader.resultCode()) + ") " +
                QString::fromStdString(reader.resultMessage());
        QMessageBox msgBox;
        msgBox.setText(errorText);
        msgBox.exec();
        return;
    }
    file.close();

    // create resource tree
    auto newTree = img::Util::createResourceNodes(*reader.format(), true);

    {
        auto& stack = mProject.commandStack();
        auto& holder = mProject.resourceHolder();

        cmnd::ScopedMacro macro(stack, "add new resource");

        // notifier
        macro.grabListener(new AddNewOneNotifier(mViaPoint, mProject));

        stack.push(new NewTreePusher(holder, newTree, aFilePath));
    }
}

void ResourceUpdater::reload(Item& aItem)
{
    auto& holder = mProject.resourceHolder();

    img::ResourceNode& node = aItem.node();
    img::ResourceNode& topNode = util::TreeUtil::getTreeRoot(node);
    QString filePath = holder.findFilePath(topNode);
    if (filePath.isEmpty()) return;

    // open file
    std::ifstream file(filePath.toLocal8Bit(), std::ios::binary);
    if (file.fail())
    {
        QMessageBox msgBox;
        msgBox.setText("can not found a resource file.");
        msgBox.exec();
        return;
    }

    // read psd
    img::PSDReader reader(file);
    if (reader.resultCode() != img::PSDReader::ResultCode_Success)
    {
        const QString errorText =
                "error(" + QString::number(reader.resultCode()) + ") " +
                QString::fromStdString(reader.resultMessage());
        QMessageBox msgBox;
        msgBox.setText(errorText);
        msgBox.exec();
        return;
    }
    file.close();

    // create resource tree
    QScopedPointer<img::ResourceNode> newTree(
                img::Util::createResourceNodes(*reader.format(), false));

    RESOURCE_UPDATER_DUMP("begin reload");

    // reload images
    if (!tryReloadCorrespondingImages(reader.format()->header(), aItem, newTree.data()))
    {
        QMessageBox msgBox;
        msgBox.setText("failed to reload images");
        msgBox.exec();
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

std::pair<bool, QVector<QString>> allChildrenCanBeIdentified(
        img::ResourceNode& aCurNode, img::ResourceNode& aNewNode)
{
    std::pair<bool, QVector<QString>> result;
    result.first = true;

    for (auto child : aNewNode.children())
    {
        auto corresponds = findCorrespondingNode(aCurNode.children(), *child);
        if (corresponds.first > 1)
        {
            result.first = false;
            result.second.push_back(child->data().identifier());
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

cmnd::Base* createImageSetter(
        img::ResourceNode& aCurNode,
        const img::PSDFormat::Header& aHeader,
        const img::PSDFormat::Layer& aLayer)
{
    auto image = img::Util::createTextureImage(aHeader, aLayer);
    return new ImageSetter(aCurNode, image.first, image.second);
}

img::ResourceNode* createNewAppendNode(
        ModificationNotifier& aNotifier,
        const img::PSDFormat::Header& aHeader,
        img::ResourceNode& aNode)
{
    using img::PSDFormat;

    auto newNode = new img::ResourceNode(aNode.data().identifier());
    newNode->data().setPos(aNode.data().pos());
    newNode->data().setIsLayer(aNode.data().isLayer());

    aNotifier.event().pushTarget(*newNode);

    if (newNode->data().isLayer())
    {
        auto newLayer = static_cast<PSDFormat::Layer*>(aNode.data().userData());
        XC_PTR_ASSERT(newLayer);
        auto image = img::Util::createTextureImage(aHeader, *newLayer);
        newNode->data().setPos(image.second.topLeft());
        newNode->data().grabImage(image.first, image.second.size(), img::Format_RGBA8);
        newNode->data().setBlendMode(img::getBlendModeFromPSD(newLayer->blendMode));
    }

    for (auto child : aNode.children())
    {
        auto newChild = createNewAppendNode(aNotifier, aHeader, *child);
        newNode->children().pushBack(newChild);
    }

    return newNode;
}

void ResourceUpdater::reloadImages(
        cmnd::Stack& aStack,
        ModificationNotifier& aNotifier,
        const img::PSDFormat::Header& aHeader,
        img::ResourceNode& aCurNode,
        img::ResourceNode& aNewNode)
{
    using img::PSDFormat;

    RESOURCE_UPDATER_DUMP("reload image %s", aCurNode.data().identifier().toLatin1().data());

    aNotifier.event().pushTarget(aCurNode);

    if (aCurNode.data().isLayer())
    {
        // reload image
        auto newLayer = static_cast<PSDFormat::Layer*>(aNewNode.data().userData());
        XC_PTR_ASSERT(newLayer);
        aStack.push(createImageSetter(aCurNode, aHeader, *newLayer));
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
            reloadImages(aStack, aNotifier, aHeader, *corresponds.second, *child);
        }
        else
        {
            // append new node
            auto newChild = createNewAppendNode(aNotifier, aHeader, *child);
            aStack.push(new cmnd::PushBackNewTreeObject<img::ResourceNode>(&aCurNode.children(), newChild));
        }
    }
}

bool ResourceUpdater::tryReloadCorrespondingImages(
        const img::PSDFormat::Header& aHeader,
        QTreeWidgetItem& aTarget,
        img::ResourceNode* aNewTree)
{
    if (!aNewTree) return false;

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

        if (corresponds.first != 1) return false;
        XC_PTR_ASSERT(corresponds.second);
        newNode = corresponds.second;
    }

    // check reloadable
    auto beIdentified = allChildrenCanBeIdentified(targetNode, *newNode);
    if (!beIdentified.first) return false;

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
        reloadImages(stack, *notifier, aHeader, targetNode, *newNode);

        // create key updating commands
        stack.push(mProject.objectTree().createResourceUpdater(notifier->event()));
    }
    return true;
}

} // namespace res
} // namespace gui
