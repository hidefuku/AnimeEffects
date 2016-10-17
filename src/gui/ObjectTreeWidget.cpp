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
#include "core/LayerSetNode.h"
#include "ctrl/TimeLineRow.h"
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
    , mInserts()
    , mMacroScope()
    , mObjTreeNotifier()
    , mDragIndex()
    , mDropIndicatorPos()
    , mActionItem()
    , mNameAction()
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
        mNameAction = new QAction("change object name", this);
        mNameAction->connect(mNameAction, &QAction::triggered, this, &ObjectTreeWidget::onNameActionTriggered);

        mObjectAction = new QAction("create image object", this);
        mObjectAction->connect(mObjectAction, &QAction::triggered, this, &ObjectTreeWidget::onObjectActionTriggered);

        mFolderAction = new QAction("create folder object", this);
        mFolderAction->connect(mFolderAction, &QAction::triggered, this, &ObjectTreeWidget::onFolderActionTriggered);

        mDeleteAction = new QAction("delete object", this);
        mDeleteAction->connect(mDeleteAction, &QAction::triggered, this, &ObjectTreeWidget::onDeleteActionTriggered);
    }
}

void ObjectTreeWidget::setProject(core::Project* aProject)
{
    if (mProject)
    {
        mProject->onTimeLineModified.disconnect(mTimeLineSlot);

        if (this->topLevelItemCount() > 0)
        {
            XC_ASSERT(this->topLevelItemCount() == 1);
            auto hook = (ProjectHook*)mProject->hook();
            hook->grabTreeRoot(this->takeTopLevelItem(0));
        }
        mProject.reset();
    }

    XC_ASSERT(this->topLevelItemCount() == 0);
    this->clear(); // fail safe code

    if (aProject)
    {
        mProject = aProject->pointee();

        mTimeLineSlot = mProject->onTimeLineModified.connect(
                    this, &ObjectTreeWidget::onTimeLineModified);

        auto hook = (ProjectHook*)mProject->hook();
        if (hook && hook->hasTreeRoot())
        {
            this->addTopLevelItem(hook->releaseTreeRoot());
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
        if (objItem)
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
            QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(node->name()));
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
    return aNode.timeLine() ?
                ctrl::TimeLineRow::calculateHeight(
                    aNode.timeLine()->validTypeCount()) :
                kItemSize;
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

void ObjectTreeWidget::storeInsertion(
        const util::TreePos& aFrom, const util::TreePos& aTo, QTreeWidgetItem* aItem)
{
    if (aFrom == aTo) return;

    if (mProject)
    {
        XC_ASSERT(aFrom.row(0) == 0 && aTo.row(0) == 0);

        // firstly, create macro
        if (!mMacroScope)
        {
            mObjTreeNotifier = new core::ObjectTreeNotifier(*mProject);
            mObjTreeNotifier->event().setType(core::ObjectTreeEvent::Type_Move);

            mMacroScope.construct(mProject->commandStack(), "change object tree");
            mMacroScope->grabListener(mObjTreeNotifier);
        }
        // record target
        auto objItem = obj::Item::cast(aItem);
        if (objItem)
        {
            core::ObjectNode& node = objItem->node();
            mObjTreeNotifier->event().pushTarget(node.parent(), node);
        }

        // push command
        mProject->commandStack().push(new obj::MoveItem(*this, aFrom, aTo));
        mProject->commandStack().push(mProject->objectTree().createNodeMover(aFrom, aTo));
    }
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

void ObjectTreeWidget::endEdit()
{
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
                node.setName(newName);
            }
        }
        mActionItem = nullptr;
    }
}

bool ObjectTreeWidget::updateItemHeights(QTreeWidgetItem* aItem)
{
    if (!aItem) return false;

    // cast to a objectnode's item
    obj::Item* item = obj::Item::cast(aItem);
    bool changed = false;

    if (item)
    {
        const int height = itemHeight(item->node());
        // update
        if (item->sizeHint(kItemColumn).height() != height)
        {
            item->setSizeHint(kItemColumn, QSize(kItemSize, height));
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
    endEdit();
#endif
}

void ObjectTreeWidget::onItemClicked(QTreeWidgetItem* aItem, int aColumn)
{
    endEdit();

    obj::Item* item = obj::Item::cast(aItem);
    if (aColumn == kItemColumn && item)
    {
        if (mProject)
        {
            const bool isVisible = item->checkState(kItemColumn) == Qt::Checked;
            item->node().setVisibility(isVisible);
            onVisibilityUpdated();
        }
    }
}

void ObjectTreeWidget::onItemCollapsed(QTreeWidgetItem* aItem)
{
    (void)aItem;
    endEdit();

    notifyViewUpdated();
}

void ObjectTreeWidget::onItemExpanded(QTreeWidgetItem* aItem)
{
    (void)aItem;
    endEdit();

    notifyViewUpdated();
}

void ObjectTreeWidget::onItemSelectionChanged()
{
    core::ObjectNode* representNode = findSelectingRepresentNode();
    onSelectionChanged(representNode);
}

void ObjectTreeWidget::onContextMenuRequested(const QPoint& aPos)
{
    endEdit();

    mActionItem = this->itemAt(aPos);
    if (mActionItem)
    {
        QMenu menu(this);

        menu.addAction(mNameAction);
        menu.addSeparator();
        menu.addAction(mObjectAction);
        menu.addAction(mFolderAction);
        menu.addSeparator();
        menu.addAction(mDeleteAction);

        menu.exec(this->mapToGlobal(aPos));
    }
}

void ObjectTreeWidget::onNameActionTriggered(bool)
{
    if (mActionItem)
    {
        this->openPersistentEditor(mActionItem, kItemColumn);
        this->editItem(mActionItem, kItemColumn);
    }
}

void ObjectTreeWidget::onObjectActionTriggered(bool)
{
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);
        if (!objItem) return;

        core::ObjectNode& node = objItem->node();
        core::ObjectNode* parent = node.parent();
        if (!parent) return;

        auto index = parent->children().indexOf(&node);
        if (index < 0) return;

        QTreeWidgetItem* parentItem = mActionItem->parent();
        if (!parentItem) return;

        const int itemIndex = parentItem->indexOfChild(objItem);
        if (itemIndex < 0) return;

        // show resource dialog
        QScopedPointer<ResourceDialog> dialog(
                    new ResourceDialog(mViaPoint, true, this));
        dialog->setProject(mProject.get());
        dialog->updateResources();
        dialog->exec();

        // create command
        if (dialog->hasValidNode())
        {
            auto resNode = dialog->nodeList().first();
            if (!resNode) return;

            XC_ASSERT(resNode->data().hasImage());
            const QRect resRect(resNode->data().pos(), resNode->data().image().pixelSize());

            cmnd::ScopedMacro macro(mProject->commandStack(), "create object");
            {
                auto coreNotifier = new core::ObjectTreeNotifier(*mProject);
                coreNotifier->event().setType(core::ObjectTreeEvent::Type_Add);
                coreNotifier->event().pushTarget(parent, node);
                macro.grabListener(coreNotifier);
            }
            macro.grabListener(new obj::RestructureNotifier(*this));

            // create node
            core::LayerNode* ptr = new core::LayerNode(
                        resNode->data().identifier(),
                        mProject->objectTree().shaderHolder());
            ptr->setDepth(node.depth() + 1.0f);
            ptr->setVisibility(true);
            ptr->setImage(resNode->handle());
            ptr->setInitialCenter(util::MathUtil::getCenter(resRect));
            mProject->commandStack().push(new cmnd::GrabNewObject<core::LayerNode>(ptr));
            mProject->commandStack().push(new cmnd::InsertTree<core::ObjectNode>(&(parent->children()), index, ptr));

            // create item
            auto itemPtr = createFileItem(*ptr);
            mProject->commandStack().push(new cmnd::GrabNewObject<obj::Item>(itemPtr));
            mProject->commandStack().push(new obj::InsertItem(*parentItem, itemIndex, *itemPtr));
        }
    }
}

void ObjectTreeWidget::onFolderActionTriggered(bool)
{
    if (mActionItem)
    {
        obj::Item* objItem = obj::Item::cast(mActionItem);
        if (!objItem) return;

        core::ObjectNode& node = objItem->node();
        core::ObjectNode* parent = node.parent();
        if (!parent) return;

        auto index = parent->children().indexOf(&node);
        if (index < 0) return;

        QTreeWidgetItem* parentItem = mActionItem->parent();
        if (!parentItem) return;

        const int itemIndex = parentItem->indexOfChild(objItem);
        if (itemIndex < 0) return;

        // create command
        {
            cmnd::ScopedMacro macro(mProject->commandStack(), "create object folder");
            {
                auto coreNotifier = new core::ObjectTreeNotifier(*mProject);
                coreNotifier->event().setType(core::ObjectTreeEvent::Type_Add);
                coreNotifier->event().pushTarget(parent, node);
                macro.grabListener(coreNotifier);
            }
            macro.grabListener(new obj::RestructureNotifier(*this));

            // create node
            core::LayerSetNode* ptr = new core::LayerSetNode("folder0");
            ptr->setDepth(node.depth() + 1.0f);
            mProject->commandStack().push(new cmnd::GrabNewObject<core::LayerSetNode>(ptr));
            mProject->commandStack().push(new cmnd::InsertTree<core::ObjectNode>(&(parent->children()), index, ptr));

            // create item
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
        if (!objItem) return;

        core::ObjectNode& node = objItem->node();

        core::ObjectNode* parent = node.parent();
        if (!parent) return;

        //auto index = parent->children().indexOf(&node);
        //if (index < 0) return;

        QTreeWidgetItem* parentItem = mActionItem->parent();
        if (!parentItem) return;

        const int itemIndex = parentItem->indexOfChild(objItem);
        if (itemIndex < 0) return;

        // delete command
        {
            cmnd::ScopedMacro macro(mProject->commandStack(), "delete object");
            {
                auto coreNotifier = new core::ObjectTreeNotifier(*mProject);
                coreNotifier->event().setType(core::ObjectTreeEvent::Type_Delete);
                coreNotifier->event().pushTarget(parent, node);
                macro.grabListener(coreNotifier);
            }
            macro.grabListener(new obj::RestructureNotifier(*this));

            // delete node
            //mProject->commandStack().push(new cmnd::RemoveTree<core::ObjectNode>(&(parent->children()), index));
            //mProject->commandStack().push(new cmnd::GrabDeleteObject<core::ObjectNode>(&node));
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
        mInserts.clear();

        QList<QTreeWidgetItem*> selected = this->selectedItems();
        for (QList<QTreeWidgetItem*>::iterator itr = selected.begin(); itr != selected.end(); ++itr)
        {
            util::TreePos pos(this->indexFromItem(*itr));
            mInserts.push_back(ItemInfo(*itr, pos));
        }

        mStoreInsert = true;
        QTreeWidget::dropEvent(&dummyEvent);
        mStoreInsert = false;

        if (mMacroScope)
        {
            mMacroScope->grabListener(new obj::RestructureNotifier(*this));
            mMacroScope.destruct();
        }
    }
    else
    {
        aEvent->ignore();
    }
}

void ObjectTreeWidget::rowsInserted(const QModelIndex& aParent, int aStart, int aEnd)
{
    if (mStoreInsert)
    {
        XC_ASSERT(aStart == aEnd);
        QTreeWidgetItem* item = this->itemFromIndex(aParent.child(aStart, kItemColumn));
        util::TreePos insertPos(this->indexFromItem(item));
        XC_ASSERT(insertPos.isValid());

        // find item
        util::TreePos removePos;
        for (size_t i = 0; i < mInserts.size(); ++i)
        {
            if (mInserts[i].ptr && mInserts[i].ptr == item)
            {
                removePos = mInserts[i].pos;
                // record insertion
                storeInsertion(removePos, insertPos, item);
                mInserts[i].ptr = nullptr;
                break;
            }
        }
        // update pos
        if (removePos.isValid())
        {
            for (size_t i = 0; i < mInserts.size(); ++i)
            {
                if (mInserts[i].ptr)
                {
                    mInserts[i].pos.updateByRemove(removePos);
                    mInserts[i].pos.updateByInsert(insertPos);
                }
            }
        }
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

} // namespace gui
