#include <QMenu>
#include <QFileDialog>
#include "util/TreeUtil.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "ctrl/CmndName.h"
#include "gui/res/res_ResourceTree.h"
#include "gui/res/res_Item.h"
#include "gui/res/res_ResourceUpdater.h"

namespace gui {
namespace res {

ResourceTree::ResourceTree(ViaPoint& aViaPoint, bool aUseCustomContext, QWidget* aParent)
    : QTreeWidget(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mHolder()
    , mActionItem()
    , mChangePathAction()
    , mReloadAction()
    , mDeleteAction()
{
    //this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setObjectName("resourceTree");
    this->setHeaderHidden(true);
    this->setAnimated(true);
    //this->setDragDropMode(DragDropMode::InternalMove);
    //this->setDefaultDropAction(Qt::TargetMoveAction);
    this->setAlternatingRowColors(true);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    this->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    this->setColumnCount(kColumnCount);
    this->setFocusPolicy(Qt::NoFocus);

    this->connect(this, &QTreeWidget::itemSelectionChanged,
                  this, &ResourceTree::onItemSelectionChanged);

    // custom context
    if (aUseCustomContext)
    {
        this->setContextMenuPolicy(Qt::CustomContextMenu);

        this->connect(this, &QWidget::customContextMenuRequested,
                      this, &ResourceTree::onContextMenuRequested);

        mChangePathAction = new QAction(tr("change file path"), this);
        mChangePathAction->connect(mChangePathAction, &QAction::triggered,
                                   this, &ResourceTree::onChangePathActionTriggered);

        mReloadAction = new QAction(tr("reload images"), this);
        mReloadAction->connect(mReloadAction, &QAction::triggered,
                               this, &ResourceTree::onReloadActionTriggered);

        mDeleteAction = new QAction(tr("delete"), this);
        mDeleteAction->connect(mDeleteAction, &QAction::triggered,
                               this, &ResourceTree::onDeleteActionTriggered);
    }
}

void ResourceTree::setProject(core::Project* aProject)
{
    mProject = aProject;
}

void ResourceTree::resetTreeView()
{
    this->clear();
    mHolder = nullptr;
}

void ResourceTree::resetTreeView(core::ResourceHolder& aHolder)
{
    this->clear();
    mHolder = &aHolder;

    for (auto data : aHolder.imageTrees())
    {
        auto item = new Item(*this, *data.topNode, data.filePath);
        this->addTopLevelItem(item);
        addTreeItemRecursive(item, data.topNode);
    }
}

QTreeWidgetItem* ResourceTree::findItem(const util::TreePos& aPos)
{
    if (!aPos.isValid() || aPos.depth() < 1) return nullptr;
    QTreeWidgetItem* item = this->topLevelItem(aPos.row(0));

    for (int i = 1; i < aPos.depth(); ++i)
    {
        if (!item) return nullptr;
        item = item->child(aPos.row(i));
    }
    return item;
}

void ResourceTree::resetTreeView(
        core::ResourceHolder& aHolder, const util::TreePos& aRoot)
{
    XC_ASSERT(aRoot.isValid());

    QTreeWidgetItem* rootItem = findItem(aRoot);
    XC_PTR_ASSERT(rootItem);

    QTreeWidgetItem* parent = rootItem->parent();
    const int index = parent ?
                parent->indexOfChild(rootItem) :
                this->indexOfTopLevelItem(rootItem);

    img::ResourceNode* rootNode = nullptr;
    {
        this->removeItemWidget(rootItem, kItemColumn);
        Item* resItem = Item::cast(rootItem);
        XC_PTR_ASSERT(resItem);
        rootNode = &(resItem->node());
        delete rootItem;
    }

    mHolder = &aHolder;

    if (parent)
    {
        auto item = new Item(*this, *rootNode, rootNode->data().identifier());
        parent->insertChild(index, item);
        addTreeItemRecursive(item, rootNode);
    }
    else
    {
        const QString filePath = aHolder.findRelativeFilePath(*rootNode);
        auto item = new Item(*this, *rootNode, filePath);
        this->insertTopLevelItem(index, item);
        addTreeItemRecursive(item, rootNode);
    }
}

void ResourceTree::addTreeItemRecursive(QTreeWidgetItem* aItem, img::ResourceNode* aNode)
{
    XC_PTR_ASSERT(aNode);
    XC_PTR_ASSERT(aItem);

    for (auto childNode : aNode->children())
    {
        auto childItem = new Item(*this, *childNode, childNode->data().identifier());
        aItem->addChild(childItem);

        // recursive call
        addTreeItemRecursive(childItem, childNode);
    }
}

void ResourceTree::updateTreeRootName(core::ResourceHolder& aHolder)
{
    if (mHolder != &aHolder) return;

    for (int i = 0; i < this->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = this->topLevelItem(i);
        XC_PTR_ASSERT(item);
        Item* resItem = Item::cast(item);
        XC_PTR_ASSERT(resItem);

        auto nodename = mHolder->findRelativeFilePath(resItem->node());
        item->setText(kItemColumn, nodename);
    }
}

QList<img::ResourceNode*> ResourceTree::findSelectingNodes() const
{
    QList<QTreeWidgetItem*> items = this->selectedItems();
    QList<img::ResourceNode*> nodes;

    for (auto item : items)
    {
        Item* resItem = res::Item::cast(item);
        if (resItem)
        {
            nodes.push_back(&resItem->node());
        }
    }
    return nodes;
}

void ResourceTree::onItemSelectionChanged()
{
    auto nodes = findSelectingNodes();
    onNodeSelectionChanged(nodes);
}

void ResourceTree::onContextMenuRequested(const QPoint& aPos)
{
    mActionItem = this->itemAt(aPos);

    if (mActionItem)
    {
        QMenu menu(this);

        Item* item = Item::cast(mActionItem);
        if (item && item->isTopNode())
        {
            menu.addAction(mChangePathAction);
            menu.addSeparator();
        }

        menu.addAction(mReloadAction);

        if (item && item->isTopNode())
        {
            menu.addSeparator();
            menu.addAction(mDeleteAction);
        }

        menu.exec(this->mapToGlobal(aPos));
    }
}

void ResourceTree::onChangePathActionTriggered(bool)
{
    Item* item = Item::cast(mActionItem);
    if (item && item->isTopNode())
    {
        const QString fileName = QFileDialog::getOpenFileName(
                    this, tr("Open File"), "", "ImageFile (*.psd *.jpg *.jpeg *.png *.gif)");
        if (fileName.isEmpty()) return;

        if (mHolder)
        {
            auto& stack = mProject->commandStack();
            cmnd::ScopedMacro macro(stack, CmndName::tr("update a resource file path"));

            // notifier
            auto notifier = new ChangeFilePathNotifier(mViaPoint, item->node());
            macro.grabListener(notifier);

            img::ResourceNode* resNode = &(item->node());
            auto absFilePath = QFileInfo(fileName).absoluteFilePath();
            auto prevFilePath = mHolder->findRelativeFilePath(item->node());

            auto command = new cmnd::Delegatable([=]()
            {
                mHolder->changeFilePath(*resNode, absFilePath);
            },
            [=]()
            {
                mHolder->changeFilePath(*resNode, prevFilePath);
            });

            stack.push(command);
        }
    }
}

void ResourceTree::onReloadActionTriggered(bool)
{
    if (!mProject) return;

    Item* item = Item::cast(mActionItem);
    if (!item) return;

    ResourceUpdater updater(mViaPoint, *mProject);
    updater.reload(*item);
}

void ResourceTree::onDeleteActionTriggered(bool)
{
    if (!mProject) return;

    Item* item = Item::cast(mActionItem);
    if (!item) return;

    ResourceUpdater updater(mViaPoint, *mProject);
    updater.remove(*item);
}

} // namespace res
} // namespace gui

