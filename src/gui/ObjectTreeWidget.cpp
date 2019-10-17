#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QDragMoveEvent>
#include <QModelIndexList>
#include <QStyle>
#include <QProxyStyle>
#include "util/TreeUtil.h"
#include "util/LinkPointer.h"
#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/LayerNode.h"
#include "core/FolderNode.h"
#include "ctrl/TimeLineRow.h"
#include "ctrl/CmndName.h"
#include "gui/ObjectTreeWidget.h"
#include "gui/ResourceDialog.h"
#include "gui/ProjectHook.h"
#include "gui/obj/obj_MoveItem.h"
#include "gui/obj/obj_InsertItem.h"
#include "gui/obj/obj_RemoveItem.h"
#include "gui/obj/obj_Notifiers.h"
#include "gui/obj/obj_Util.h"

namespace
{
static const int kTopItemSize = 22;
static const int kItemSize = ctrl::TimeLineRow::kHeight;
static const int kItemSizeInc = ctrl::TimeLineRow::kIncrease;
}

namespace gui
{

//-------------------------------------------------------------------------------------------------
ObjectTreeWidget::ObjectTreeWidget(ViaPoint& aViaPoint, GUIResources& aResources, QWidget* aParent)
    : QTreeWidget(aParent)
    , mViaPoint(aViaPoint)
    , mResources(aResources)
    , mProject()
    , mTimeLineSlot()
    , mStoreInsert(false)
    , mRemovedPositions()
	, mInsertedPositions()
    , mMacroScope()
    , mObjTreeNotifier()
    , mDragIndex()
    , mDropIndicatorPos()
    , mActionItem()
    , mSlimAction()
    , mRenameAction()
    , mObjectAction()
    , mFolderAction()
    , mDeleteAction()
{
    {
        QFile stylesheet("data/stylesheet/standard.ssa");
        if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->setStyleSheet(QTextStream(&stylesheet).readAll());
        }
    }

    this->setObjectName("objectTree");
    this->setFocusPolicy(Qt::NoFocus);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setHeaderHidden(true);
    this->setAnimated(true);
    this->setUniformRowHeights(false);
    this->setDragDropMode(DragDropMode::InternalMove);
    this->setDefaultDropAction(Qt::TargetMoveAction);
    //this->setAlternatingRowColors(true);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    this->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    //this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    //this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setColumnCount(kColumnCount);

    if (this->invisibleRootItem())
    {
        this->invisibleRootItem()->setFlags(Qt::NoItemFlags);
    }
    this->connect(this, &QWidget::customContextMenuRequested, this, &ObjectTreeWidget::onContextMenuRequested);
    this->connect(this, &QTreeWidget::itemChanged, this, &ObjectTreeWidget::onItemChanged);
    this->connect(this, &QTreeWidget::itemClicked, this, &ObjectTreeWidget::onItemClicked);
    this->connect(this, &QTreeWidget::itemCollapsed, this, &ObjectTreeWidget::onItemCollapsed);
    this->connect(this, &QTreeWidget::itemExpanded, this, &ObjectTreeWidget::onItemExpanded);
    this->connect(this, &QTreeWidget::itemSelectionChanged, this, &ObjectTreeWidget::onItemSelectionChanged);

    {
        mSlimAction = new QAction(tr("slim down"), this);
        mSlimAction->connect(mSlimAction, &QAction::triggered, this, &ObjectTreeWidget::onSlimActionTriggered);

        mRenameAction = new QAction(tr("rename"), this);
        mRenameAction->connect(mRenameAction, &QAction::triggered, this, &ObjectTreeWidget::onRenameActionTriggered);

        mObjectAction = new QAction(tr("create layer object"), this);
        mObjectAction->connect(mObjectAction, &QAction::triggered, this, &ObjectTreeWidget::onObjectActionTriggered);

        mFolderAction = new QAction(tr("create folder object"), this);
        mFolderAction->connect(mFolderAction, &QAction::triggered, this, &ObjectTreeWidget::onFolderActionTriggered);

        mDeleteAction = new QAction(tr("delete"), this);
        mDeleteAction->connect(mDeleteAction, &QAction::triggered, this, &ObjectTreeWidget::onDeleteActionTriggered);
    }
}

void ObjectTreeWidget::setProject(core::Project* aProject)
{
    // finalize
    if (mProject)
    {
        mProject->onTimeLineModified.disconnect(mTimeLineSlot);

        auto treeCount = this->topLevelItemCount();
        if (treeCount > 0)
        {
            QScopedPointer<QVector<QTreeWidgetItem*>> trees(
                        new QVector<QTreeWidgetItem*>());
            for (int i = 0; i < treeCount; ++i)
            {
                trees->push_back(this->takeTopLevelItem(0));
            }
            // save
			auto hook = static_cast<ProjectHook*>(mProject->hook());
            hook->grabObjectTrees(trees.take());
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

    // setup
    if (mProject)
    {

        mTimeLineSlot = mProject->onTimeLineModified.connect(
                    this, &ObjectTreeWidget::onTimeLineModified);

		auto hook = static_cast<ProjectHook*>(mProject->hook());
        // load trees
        if (hook && hook->hasObjectTrees())
        {
            QScopedPointer<QVector<QTreeWidgetItem*>> trees(
                        hook->releaseObjectTrees());
            for (auto tree : *trees)
            {
                this->addTopLevelItem(tree);
            }
            trees.reset();
        }
        else
        {
            createTree(&(mProject->objectTree()));
        }
    }
    notifyViewUpdated();
}

core::ObjectNode* ObjectTreeWidget::findSelectingRepresentNode()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    core::ObjectNode* node = nullptr;

    for (auto item : items)
    {
        obj::Item* objItem = obj::Item::cast(item);
        if (objItem && !objItem->isTopNode())
        {
            if (node) return nullptr;
            node = &objItem->node();
        }
    }
    return node;
}

void ObjectTreeWidget::notifyViewUpdated()
{
    onTreeViewUpdated(this->topLevelItem(0));
    onScrollUpdated(scrollHeight());
}

void ObjectTreeWidget::notifyRestructure()
{
    onTreeViewUpdated(this->topLevelItem(0));
    onScrollUpdated(scrollHeight());
}

//-------------------------------------------------------------------------------------------------
void ObjectTreeWidget::createTree(core::ObjectTree* aTree)
{
    this->clear();

    if (aTree)
    {
        core::ObjectNode* node = aTree->topNode();
        if (node)
        {
            obj::Item* item = new obj::Item(*this, *node);
            item->setSizeHint(kItemColumn, QSize(kTopItemSize, kTopItemSize));
            this->addTopLevelItem(item);
            addItemRecursive(item, node);
        }
    }
    notifyViewUpdated();
}

void ObjectTreeWidget::addItemRecursive(QTreeWidgetItem* aItem, core::ObjectNode* aNode)
{
    const core::ObjectNode::Children& children = aNode->children();
    for (auto childNode : children)
    {
        if (childNode->canHoldChild())
        {
            auto childItem = createFolderItem(*childNode);
            aItem->addChild(childItem);
            addItemRecursive(childItem, childNode);
        }
        else
        {
            aItem->addChild(createFileItem(*childNode));
        }
    }
}

int ObjectTreeWidget::itemHeight(const core::ObjectNode& aNode) const
{
    return ctrl::TimeLineRow::calculateHeight(aNode);
}

obj::Item* ObjectTreeWidget::createFolderItem(core::ObjectNode& aNode)
{

    obj::Item* item = new obj::Item(*this, aNode);
    item->setSizeHint(kItemColumn, QSize(kItemSize, itemHeight(aNode)));
    item->setBackgroundColor(kItemColumn, QColor(235, 235, 235, 255));
    item->setIcon(kItemColumn, mResources.icon("folder"));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(kItemColumn, aNode.isVisible() ? Qt::Checked : Qt::Unchecked);
    return item;
}

obj::Item* ObjectTreeWidget::createFileItem(core::ObjectNode& aNode)
{
    obj::Item* item = new obj::Item(*this, aNode);
    item->setSizeHint(kItemColumn, QSize(kItemSize, itemHeight(aNode)));
    item->setIcon(kItemColumn, mResources.icon("filew"));
    item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(kItemColumn, aNode.isVisible() ? Qt::Checked : Qt::Unchecked);
    return item;
}

QModelIndex ObjectTreeWidget::cheatDragDropPos(QPoint& aPos)
{
    static const int kMargin = 5;

    QModelIndex index = this->indexAt(aPos);
    if (index.isValid())
    {
        QRect rect = this->visualRect(index);
        if (aPos.y() - rect.top() < kMargin)
        {
            aPos.setY(rect.top() + 1);
        }
        if (rect.bottom() - aPos.y() < kMargin)
        {
            aPos.setY(rect.bottom() - 1);
        }
    }
    return index;
}

QPoint ObjectTreeWidget::treeTopLeftPosition() const
{
    if (topLevelItemCount())
    {
        return visualItemRect(topLevelItem(0)).topLeft();
    }
    return QPoint();
}

void ObjectTreeWidget::endRenameEditor()
{
    class NameChanger : public cmnd::Stable
    {
        core::ObjectNode& mNode;
        QTreeWidgetItem& mItem;
        QString mPrevName;
        QString mNextName;
    public:
        NameChanger(core::ObjectNode& aNode, QTreeWidgetItem& aItem, const QString& aName)
            : mNode(aNode)
            , mItem(aItem)
            , mPrevName()
            , mNextName(aName)
        {
        }

        virtual QString name() const
        {
            return CmndName::tr("rename a object");
        }

        virtual void exec()
        {
            mPrevName = mNode.name();
            redo();
        }

        virtual void redo()
        {
            mNode.setName(mNextName);
            mItem.setText(kItemColumn, mNextName);
        }

        virtual void undo()
        {
            mNode.setName(mPrevName);
            mItem.setText(kItemColumn, mPrevName);
        }
    };

    if (mActionItem)
    {
        this->closePersistentEditor(mActionItem, kItemColumn);

        obj::Item* objItem = obj::Item::cast(mActionItem);
        if (objItem)
        {
            core::ObjectNode& node = objItem->node();
            const QString newName = objItem->text(kItemColumn);
            if (node.name() != newName)
            {
                mProject->commandStack().push(new NameChanger(node, *mActionItem, newName));
            }
        }
        mActionItem = nullptr;
    }
}

bool ObjectTreeWidget::updateItemHeights(QTreeWidgetItem* aItem)
{
    if (!aItem) return false;

    // cast to a objectnode's item
    obj::Item* objItem = obj::Item::cast(aItem);
    bool changed = false;

    if (objItem && !objItem->isTopNode())
    {
        const int height = itemHeight(objItem->node());
        // update
        if (objItem->sizeHint(kItemColumn).height() != height)
        {
            objItem->setSizeHint(kItemColumn, QSize(kItemSize, height));
            changed = true;
        }
    }

    // recursive call
    const int childCount = aItem->childCount();
    for (int i = 0; i < childCount; ++i)
    {
        changed |= updateItemHeights(aItem->child(i));
    }
    return changed;
}

//-------------------------------------------------------------------------------------------------
void ObjectTreeWidget::onTimeLineModified(core::TimeLineEvent& aEvent, bool)
{
    auto type = aEvent.type();
    if (type != core::TimeLineEvent::Type_PushKey &&
        type != core::TimeLineEvent::Type_RemoveKey)
    {
        return;
    }

    // update height of items in proportion to the key type count
    if (updateItemHeights(this->topLevelItem(0)))
    {
        notifyViewUpdated();
    }
}

void ObjectTreeWidget::onItemChanged(QTreeWidgetItem* aItem, int aColumn)
{
#if 0
    if (aColumn == kItemColumn)
    {
        this->closePersistentEditor(aItem, aColumn);
    }
#else
    // is this ok?
    (void)aItem;
    (void)aColumn;
    endRenameEditor();
#endif
}

void ObjectTreeWidget::onItemClicked(QTreeWidgetItem* aItem, int aColumn)
{
    endRenameEditor();

    obj::Item* objItem = obj::Item::cast(aItem);
    if (aColumn == kItemColumn && objItem && !objItem->isTopNode())
    {
        if (mProject)
        {
            const bool isVisible = objItem->checkState(kItemColumn) == Qt::Checked;
            objItem->node().setVisibility(isVisible);
            onVisibilityUpdated();
        }
    }
}

void ObjectTreeWidget::onItemCollapsed(QTreeWidgetItem* aItem)
{
    (void)aItem;
    endRenameEditor();

    notifyViewUpdated();
}

void ObjectTreeWidget::onItemExpanded(QTreeWidgetItem* aItem)
{
    (void)aItem;
    endRenameEditor();

    notifyViewUpdated();
}

void ObjectTreeWidget::onItemSelectionChanged()
{
    core::ObjectNode* representNode = findSelectingRepresentNode();
    onSelectionChanged(representNode);
}

void ObjectTreeWidget::onContextMenuRequested(const QPoint& aPos)
{
    endRenameEditor();

    mActionItem = this->itemAt(aPos);
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);
        QMenu menu(this);

        if (objItem && !objItem->isTopNode())
        {
            mSlimAction->setText(
                        objItem->node().isSlimmedDown() ?
                            tr("fatten") : tr("slim down"));
            menu.addAction(mSlimAction);
            menu.addSeparator();
        }
        menu.addAction(mRenameAction);
        menu.addSeparator();
        menu.addAction(mObjectAction);
        menu.addAction(mFolderAction);

        {
            if (objItem && objItem->node().parent())
            {
                menu.addSeparator();
                menu.addAction(mDeleteAction);
            }
        }

        menu.exec(this->mapToGlobal(aPos));
    }
}

void ObjectTreeWidget::onSlimActionTriggered(bool)
{
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);
        if (objItem && !objItem->isTopNode())
        {
            objItem->node().setSlimDown(!objItem->node().isSlimmedDown());

            if (updateItemHeights(this->topLevelItem(0)))
            {
                notifyViewUpdated();
            }
        }
    }
}

void ObjectTreeWidget::onRenameActionTriggered(bool)
{
    if (mActionItem)
    {
        if (obj::Item::cast(mActionItem))
        {
            this->openPersistentEditor(mActionItem, kItemColumn);
            this->editItem(mActionItem, kItemColumn);
        }
    }
}

void ObjectTreeWidget::onObjectActionTriggered(bool)
{
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);

        core::ObjectNode* parent = nullptr;
        int index = -1;
        float depth = 0.0f;

        QTreeWidgetItem* parentItem = nullptr;
        int itemIndex = -1;

        // top node
        if (!objItem || objItem->isTopNode())
        {
            parent = mProject->objectTree().topNode();
            XC_PTR_ASSERT(parent);

			index = static_cast<int>(parent->children().size());
            if (index > 0)
            {
                auto prevNode = parent->children().back();
                depth = prevNode->initialDepth() - 1.0f;
            }
            parentItem = mActionItem;
            itemIndex = parentItem->childCount();
        }
        else // sub node
        {
            auto prevNode = &(objItem->node());
            depth = prevNode->initialDepth() + 1.0f;

            parent = prevNode->parent();
            if (!parent) return;

            index = parent->children().indexOf(prevNode);
            if (index < 0) return;

            parentItem = mActionItem->parent();
            if (!parentItem) return;

            itemIndex = parentItem->indexOfChild(objItem);
            if (itemIndex < 0) return;
        }

        // show resource dialog
        QScopedPointer<ResourceDialog> dialog(
                    new ResourceDialog(mViaPoint, true, this));
        dialog->setProject(mProject.get());
        dialog->exec();

        // create command
        if (dialog->hasValidNode())
        {
			int node_count = dialog->nodeList().count();
			for(int i = 0; i < node_count; i++){
				// get resource
				auto resNode = dialog->nodeList().at(i);
				if (!resNode) return;
				XC_ASSERT(resNode->data().hasImage());
	
				// create node
				core::LayerNode* ptr = new core::LayerNode(
							resNode->data().identifier(),
							mProject->objectTree().shaderHolder());
				ptr->setVisibility(true);
				ptr->setDefaultImage(resNode->handle());
				ptr->setDefaultPosture(QVector2D());
				ptr->setDefaultDepth(depth);
				ptr->setDefaultOpacity(1.0f); // @todo support default opacity
	
	
				cmnd::ScopedMacro macro(mProject->commandStack(),
										CmndName::tr("create a layer object"));
				// notifier
				{
					auto coreNotifier = new core::ObjectTreeNotifier(*mProject);
					coreNotifier->event().setType(core::ObjectTreeEvent::Type_Add);
					coreNotifier->event().pushTarget(parent, *ptr);
					macro.grabListener(coreNotifier);
				}
				macro.grabListener(new obj::RestructureNotifier(*this));
	
				// create commands
				mProject->commandStack().push(new cmnd::GrabNewObject<core::LayerNode>(ptr));
				mProject->commandStack().push(new cmnd::InsertTree<core::ObjectNode>(&(parent->children()), index, ptr));
	
				// create gui commands
				auto itemPtr = createFileItem(*ptr);
				mProject->commandStack().push(new cmnd::GrabNewObject<obj::Item>(itemPtr));
				mProject->commandStack().push(new obj::InsertItem(*parentItem, itemIndex, *itemPtr));
			}
        }
    }
}

void ObjectTreeWidget::onFolderActionTriggered(bool)
{
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);

        core::ObjectNode* parent = nullptr;
        int index = -1;
        float depth = 0.0f;

        QTreeWidgetItem* parentItem = nullptr;
        int itemIndex = -1;

        // top node
        if (!objItem || objItem->isTopNode())
        {
            parent = mProject->objectTree().topNode();
            XC_PTR_ASSERT(parent);

			index = static_cast<int>(parent->children().size());
            if (index > 0)
            {
                auto prevNode = parent->children().back();
                depth = prevNode->initialDepth() - 1.0f;
            }
            parentItem = mActionItem;
            itemIndex = parentItem->childCount();
        }
        else // sub node
        {
            auto prevNode = &(objItem->node());
            depth = prevNode->initialDepth() + 1.0f;

            parent = prevNode->parent();
            if (!parent) return;

            index = parent->children().indexOf(prevNode);
            if (index < 0) return;

            parentItem = mActionItem->parent();
            if (!parentItem) return;

            itemIndex = parentItem->indexOfChild(objItem);
            if (itemIndex < 0) return;
        }

        // create command
        {
            cmnd::ScopedMacro macro(mProject->commandStack(),
                                    CmndName::tr("create a folder object"));

            // create node
            core::FolderNode* ptr = new core::FolderNode("folder0");
            ptr->setDefaultPosture(QVector2D());
            ptr->setDefaultDepth(depth);
            ptr->setDefaultOpacity(1.0f); // @todo support default opacity

            // notifier
            {
                auto coreNotifier = new core::ObjectTreeNotifier(*mProject);
                coreNotifier->event().setType(core::ObjectTreeEvent::Type_Add);
                coreNotifier->event().pushTarget(parent, *ptr);
                macro.grabListener(coreNotifier);
            }
            macro.grabListener(new obj::RestructureNotifier(*this));

            // push commands
            mProject->commandStack().push(new cmnd::GrabNewObject<core::FolderNode>(ptr));
            mProject->commandStack().push(new cmnd::InsertTree<core::ObjectNode>(&(parent->children()), index, ptr));

            // push gui item commands
            auto itemPtr = createFolderItem(*ptr);
            mProject->commandStack().push(new cmnd::GrabNewObject<obj::Item>(itemPtr));
            mProject->commandStack().push(new obj::InsertItem(*parentItem, itemIndex, *itemPtr));
        }
    }
}

void ObjectTreeWidget::onDeleteActionTriggered(bool)
{
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);
        if (!objItem || objItem->isTopNode()) return;

        core::ObjectNode& node = objItem->node();

        core::ObjectNode* parent = node.parent();
        if (!parent) return;

        QTreeWidgetItem* parentItem = mActionItem->parent();
        if (!parentItem) return;

        const int itemIndex = parentItem->indexOfChild(objItem);
        if (itemIndex < 0) return;

        // delete command
        {
            cmnd::ScopedMacro macro(mProject->commandStack(), CmndName::tr("delete a object"));
            {
                auto coreNotifier = new core::ObjectTreeNotifier(*mProject);
                coreNotifier->event().setType(core::ObjectTreeEvent::Type_Delete);
                coreNotifier->event().pushTarget(parent, node);
                macro.grabListener(coreNotifier);
            }
            macro.grabListener(new obj::RestructureNotifier(*this));

            // delete node
            mProject->commandStack().push(mProject->objectTree().createNodeDeleter(node));

            // delete item
            mProject->commandStack().push(new obj::RemoveItem(*parentItem, itemIndex, *objItem));
            mProject->commandStack().push(new cmnd::GrabDeleteObject<obj::Item>(objItem));
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ObjectTreeWidget::paintEvent(QPaintEvent* aEvent)
{
    QTreeWidget::paintEvent(aEvent);

    if (mDragIndex.isValid())
    {
        QPainter painter(this->viewport());

        const QTreeWidgetItem* item = this->itemFromIndex(mDragIndex);
        QRect itemRect = this->visualItemRect(item);

        const QBrush kBrush(QColor(140, 140, 140, 80));
        const QBrush kPenBrush(QColor(100, 100, 100, 200));
        painter.setBrush(kBrush);
        painter.setPen(QPen(kPenBrush, 1));

        QPoint pos;
        if (mDropIndicatorPos == QAbstractItemView::AboveItem)
        {
            pos = itemRect.topLeft();
            painter.drawLine(pos, pos + QPoint(itemRect.width(), 0));
        }
        else if (mDropIndicatorPos == QAbstractItemView::OnItem)
        {
            pos = QPoint(itemRect.left(), itemRect.center().y());
            painter.drawRect(itemRect);
        }
        else if (mDropIndicatorPos == QAbstractItemView::BelowItem)
        {
            pos = QPoint(itemRect.left(), itemRect.bottom() + 1);
            painter.drawLine(pos, pos + QPoint(itemRect.width(), 0));
        }
        QPolygon arrow;
        arrow.push_back(pos);
        arrow.push_back(pos + QPoint(-6, -4));
        arrow.push_back(pos + QPoint(-6, +4));
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawConvexPolygon(arrow);
        painter.end();
    }
}

void ObjectTreeWidget::showEvent(QShowEvent* aEvent)
{
    QTreeWidget::showEvent(aEvent);

    if (this->horizontalScrollBar())
    {
        this->setViewportMargins(0, 0, 0, this->horizontalScrollBar()->sizeHint().height());
    }
}

void ObjectTreeWidget::dragMoveEvent(QDragMoveEvent* aEvent)
{
    QPoint cheatPos = aEvent->pos();
    mDragIndex = cheatDragDropPos(cheatPos);

    QDragMoveEvent dummyEvent(cheatPos, aEvent->dropAction(), aEvent->mimeData(), aEvent->mouseButtons(), aEvent->keyboardModifiers(), aEvent->type());
    QTreeWidget::dragMoveEvent(&dummyEvent);

    if (!dummyEvent.isAccepted())
    {
        mDragIndex = QModelIndex();
        mDropIndicatorPos = QTreeWidget::DropIndicatorPosition();
        aEvent->ignore();
    }
    else
    {
        aEvent->accept();
        mDropIndicatorPos = this->dropIndicatorPosition();
    }
}

void ObjectTreeWidget::dropEvent(QDropEvent* aEvent)
{
    mDragIndex = QModelIndex();
    QPoint cheatPos = aEvent->pos();
    cheatDragDropPos(cheatPos);
    QDropEvent dummyEvent(cheatPos, aEvent->dropAction(), aEvent->mimeData(), aEvent->mouseButtons(), aEvent->keyboardModifiers(), aEvent->type());
    QModelIndex cursorIndex = this->indexAt(aEvent->pos());

    if (this->visualRect(cursorIndex).contains(aEvent->pos()))
    {
        mRemovedPositions.clear();
        mInsertedPositions.clear();

        // begin move
        mStoreInsert = true;
        QTreeWidget::dropEvent(&dummyEvent);
        mStoreInsert = false;

        // finalize move command
        if (mMacroScope)
        {
            // item mover
            mProject->commandStack().push(
                        new obj::MoveItems(
                            *this, mRemovedPositions, mInsertedPositions));

            // node mover
            mProject->commandStack().push(
                        mProject->objectTree().createNodesMover(
                            mRemovedPositions, mInsertedPositions));

            mMacroScope->grabListener(new obj::RestructureNotifier(*this));
            mMacroScope.destruct();
        }
    }
    else
    {
        aEvent->ignore();
    }
}

void ObjectTreeWidget::rowsAboutToBeRemoved(const QModelIndex& aParent, int aStart, int aEnd)
{
    if (mStoreInsert)
    {
        XC_ASSERT(aStart == aEnd);
        QTreeWidgetItem* item = this->itemFromIndex(aParent.child(aStart, kItemColumn));
        util::TreePos removePos(this->indexFromItem(item));
        XC_ASSERT(removePos.isValid());
        //qDebug() << "remove"; removePos.dump();

        mRemovedPositions.push_back(removePos);

        // firstly, create macro
        if (mProject && !mMacroScope)
        {
            mObjTreeNotifier = new core::ObjectTreeNotifier(*mProject);
            mObjTreeNotifier->event().setType(core::ObjectTreeEvent::Type_Move);

            mMacroScope.construct(mProject->commandStack(), CmndName::tr("move a object"));
            mMacroScope->grabListener(mObjTreeNotifier);
        }
        // record target
        if (mMacroScope)
        {
            auto objItem = obj::Item::cast(item);
            if (objItem)
            {
                core::ObjectNode& node = objItem->node();
                mObjTreeNotifier->event().pushTarget(node.parent(), node);
                //qDebug() << "node" << node.name() << (node.parent() ? node.parent()->name() : "");
            }
        }
    }
    QTreeWidget::rowsAboutToBeRemoved(aParent, aStart, aEnd);
}

void ObjectTreeWidget::rowsInserted(const QModelIndex& aParent, int aStart, int aEnd)
{
    if (mStoreInsert)
    {
        XC_ASSERT(aStart == aEnd);
        QTreeWidgetItem* item = this->itemFromIndex(aParent.child(aStart, kItemColumn));
        util::TreePos insertPos(this->indexFromItem(item));
        XC_ASSERT(insertPos.isValid());
        //qDebug() << "insert"; insertPos.dump();

        mInsertedPositions.push_back(insertPos);
    }
    QTreeWidget::rowsInserted(aParent, aStart, aEnd);
}

void ObjectTreeWidget::scrollContentsBy(int aDx, int aDy)
{
    QTreeWidget::scrollContentsBy(aDx, aDy);
    onScrollUpdated(scrollHeight());
}

void ObjectTreeWidget::resizeEvent(QResizeEvent* aEvent)
{
    QTreeWidget::resizeEvent(aEvent);
    onScrollUpdated(scrollHeight());
}

void ObjectTreeWidget::scrollTo(const QModelIndex& aIndex, ScrollHint aHint)
{
    // Avoided qt bug that QTreeView scroll incorrectly (in scrollTo, EnsureVisible).
    if (aHint == ScrollHint::EnsureVisible)
    {
        auto view = this->viewport()->rect();
        auto rect = this->visualRect(aIndex);
        if (view.top() <= rect.top() && rect.bottom() <= view.bottom())
        {
            return; // nothing to do
        }
    }
    QTreeWidget::scrollTo(aIndex, aHint);
}

} // namespace gui
