#include <QMenu>
#include <QFileDialog>
#include "util/TreeUtil.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "ctrl/CmndName.h"
#include "gui/ProjectHook.h"
#include "gui/ResourceTreeWidget.h"
#include "gui/res/res_Item.h"
#include "gui/res/res_ResourceUpdater.h"
#include "gui/res/res_Notifier.h"

namespace gui {

ResourceTreeWidget::ResourceTreeWidget(ViaPoint& aViaPoint, bool aUseCustomContext, QWidget* aParent)
    : QTreeWidget(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mHolder()
    , mActionItem()
    , mChangePathAction()
    , mRenameAction()
    , mReloadAction()
    , mDeleteAction()
    , mRenaming()
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
                  this, &ResourceTreeWidget::onItemSelectionChanged);

    this->connect(this, &QTreeWidget::itemChanged, [=](bool){ this->endRenameEditor(); });
    this->connect(this, &QTreeWidget::itemClicked, [=](bool){ this->endRenameEditor(); });
    this->connect(this, &QTreeWidget::itemCollapsed, [=](bool){ this->endRenameEditor(); });
    this->connect(this, &QTreeWidget::itemExpanded, [=](bool){ this->endRenameEditor(); });

    // custom context
    if (aUseCustomContext)
    {
        this->setContextMenuPolicy(Qt::CustomContextMenu);

        this->connect(this, &QWidget::customContextMenuRequested,
                      this, &ResourceTreeWidget::onContextMenuRequested);

        mChangePathAction = new QAction(tr("change file path"), this);
        mChangePathAction->connect(mChangePathAction, &QAction::triggered,
                                   this, &ResourceTreeWidget::onChangePathActionTriggered);

        mRenameAction = new QAction(tr("rename"), this);
        mRenameAction->connect(mRenameAction, &QAction::triggered,
                               this, &ResourceTreeWidget::onRenameActionTriggered);

        mReloadAction = new QAction(tr("reload images"), this);
        mReloadAction->connect(mReloadAction, &QAction::triggered,
                               this, &ResourceTreeWidget::onReloadActionTriggered);

        mDeleteAction = new QAction(tr("delete"), this);
        mDeleteAction->connect(mDeleteAction, &QAction::triggered,
                               this, &ResourceTreeWidget::onDeleteActionTriggered);
    }
}

void ResourceTreeWidget::setProject(core::Project* aProject)
{
    // finalize tree
    if (mProject)
    {
        auto treeCount = this->topLevelItemCount();
        if (treeCount > 0)
        {
            // create vector
            QScopedPointer<QVector<QTreeWidgetItem*>> trees(
                        new QVector<QTreeWidgetItem*>());
            for (int i = 0; i < treeCount; ++i)
            {
                trees->push_back(this->takeTopLevelItem(0));
            }
            // save
            auto hook = (ProjectHook*)mProject->hook();
            hook->grabResourceTrees(trees.take());
        }
    }
    XC_ASSERT(this->topLevelItemCount() == 0);
    this->clear(); // fail safe code

    // update reference
    if (aProject)
    {
        mProject = aProject->pointee();
    }
    else
    {
        mProject.reset();
    }

    // setup tree
    if (mProject)
    {
        auto hook = (ProjectHook*)mProject->hook();
        // load trees
        if (hook && hook->hasResourceTrees())
        {
            QScopedPointer<QVector<QTreeWidgetItem*>> trees(
                        hook->releaseResourceTrees());
            for (auto tree : *trees)
            {
                this->addTopLevelItem(tree);
            }
            trees.reset();
        }
        else
        {
            // create trees
            resetTreeView(mProject->resourceHolder());
        }
    }
}

void ResourceTreeWidget::load(const QString& aFileName)
{
    res::ResourceUpdater updater(mViaPoint, *mProject);
    updater.load(*this, aFileName);
}

void ResourceTreeWidget::resetTreeView(core::ResourceHolder& aHolder)
{
    this->clear();
    mHolder = &aHolder;

    for (auto data : aHolder.imageTrees())
    {
        auto item = res::ResourceUpdater::createGUITree(*this, *data.topNode, data.filePath);
        XC_PTR_ASSERT(item);
        this->addTopLevelItem(item);
    }
}

QTreeWidgetItem* ResourceTreeWidget::findItem(const util::TreePos& aPos)
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

void ResourceTreeWidget::updateTreeRootName(core::ResourceHolder& aHolder)
{
    if (mHolder != &aHolder) return;

    for (int i = 0; i < this->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = this->topLevelItem(i);
        XC_PTR_ASSERT(item);
        res::Item* resItem = res::Item::cast(item);
        XC_PTR_ASSERT(resItem);

        auto nodename = mHolder->findRelativeFilePath(resItem->node());
        item->setText(kItemColumn, nodename);
    }
}

QList<img::ResourceNode*> ResourceTreeWidget::findSelectingNodes() const
{
    QList<QTreeWidgetItem*> items = this->selectedItems();
    QList<img::ResourceNode*> nodes;

    for (auto item : items)
    {
        res::Item* resItem = res::Item::cast(item);
        if (resItem)
        {
            nodes.push_back(&resItem->node());
        }
    }
    return nodes;
}

void ResourceTreeWidget::onItemSelectionChanged()
{
    auto nodes = findSelectingNodes();
    onNodeSelectionChanged(nodes);
}

void ResourceTreeWidget::onContextMenuRequested(const QPoint& aPos)
{
    endRenameEditor();

    mActionItem = this->itemAt(aPos);

    if (mActionItem)
    {
        QMenu menu(this);

        res::Item* item = res::Item::cast(mActionItem);
        if (item && item->isTopNode())
        {
            menu.addAction(mChangePathAction);
        }
        else
        {
            menu.addAction(mRenameAction);
        }
        menu.addSeparator();

        menu.addAction(mReloadAction);

        if (item && item->isTopNode())
        {
            menu.addSeparator();
            menu.addAction(mDeleteAction);
        }

        menu.exec(this->mapToGlobal(aPos));
    }
}

void ResourceTreeWidget::onChangePathActionTriggered(bool)
{
    res::Item* item = res::Item::cast(mActionItem);
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
            auto notifier = new res::ChangeFilePathNotifier(mViaPoint, item->node());
            macro.grabListener(notifier);

            img::ResourceNode* resNode = &(item->node());
            auto absFilePath = QFileInfo(fileName).absoluteFilePath();
            auto prevFilePath = mHolder->findAbsoluteFilePath(item->node());

            auto command = new cmnd::Delegatable([=]()
            {
                mHolder->changeAbsoluteFilePath(*resNode, absFilePath);
                item->setText(kItemColumn, mHolder->relativeFilePath(absFilePath));
            },
            [=]()
            {
                mHolder->changeAbsoluteFilePath(*resNode, prevFilePath);
                item->setText(kItemColumn, mHolder->relativeFilePath(prevFilePath));
            });

            stack.push(command);
        }
    }
}

void ResourceTreeWidget::onRenameActionTriggered(bool)
{
    if (mActionItem)
    {
        this->openPersistentEditor(mActionItem, kItemColumn);
        this->editItem(mActionItem, kItemColumn);
        mRenaming = true;
    }
}

void ResourceTreeWidget::endRenameEditor()
{
    auto actionItem = mActionItem;
    mActionItem = nullptr;

    if (!mProject) return;

    if (actionItem && mRenaming)
    {
        res::Item* item = res::Item::cast(actionItem);
        if (!item) return;

        mRenaming = false;
        this->closePersistentEditor(actionItem, kItemColumn);
        auto nodePtr = &(item->node());
        auto curName = nodePtr->data().identifier();
        auto newName = item->text(kItemColumn);

        if (curName != newName)
        {
            auto& stack = mProject->commandStack();
            cmnd::ScopedMacro macro(stack, CmndName::tr("rename a resource"));

            // notifier
            auto notifier = new res::RenameNotifier(mViaPoint, *mProject, item->treePos());
            notifier->event().setSingleTarget(*nodePtr);
            macro.grabListener(notifier);

            stack.push(new cmnd::Delegatable([=]()
            {
                item->setText(kItemColumn, newName);
                nodePtr->data().setIdentifier(newName);
            }, [=]()
            {
                item->setText(kItemColumn, curName);
                nodePtr->data().setIdentifier(curName);
            }));
        }
    }
}

void ResourceTreeWidget::onReloadActionTriggered(bool)
{
    if (!mProject) return;

    res::Item* item = res::Item::cast(mActionItem);
    if (!item) return;

    res::ResourceUpdater updater(mViaPoint, *mProject);
    updater.reload(*item);
}

void ResourceTreeWidget::onDeleteActionTriggered(bool)
{
    if (!mProject) return;

    res::Item* item = res::Item::cast(mActionItem);
    if (!item) return;

    res::ResourceUpdater updater(mViaPoint, *mProject);
    updater.remove(*this, *item);
}

} // namespace gui

