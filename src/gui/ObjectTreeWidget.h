#ifndef GUI_OBJECTTREEWIDGET_H
#define GUI_OBJECTTREEWIDGET_H

#include <QTreeWidget>
#include <QDropEvent>
#include <QHeaderView>
#include <QAction>
#include "util/PlacePointer.h"
#include "util/LinkPointer.h"
#include "util/TreePos.h"
#include "util/Signaler.h"
#include "cmnd/ScopedMacro.h"
#include "core/Project.h"
#include "core/ObjectTree.h"
#include "core/ObjectTreeNotifier.h"
#include "gui/GUIResources.h"
#include "gui/ViaPoint.h"
#include "gui/obj/obj_Item.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
class ObjectTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    enum { kItemColumn = 0 };
    enum { kColumnCount = 1 };

    ObjectTreeWidget(ViaPoint& aViaPoint, GUIResources& aResources, QWidget* aParent);

    void setProject(core::Project* aProject);
    core::ObjectNode* findSelectingRepresentNode();

public:
    // signals
    util::Signaler<void()> onVisibilityUpdated;
    util::Signaler<void(QTreeWidgetItem*)> onTreeViewUpdated;
    util::Signaler<void(core::ObjectNode*)> onSelectionChanged;
    util::Signaler<void(int)> onScrollUpdated;

    // for Notifiers
    void notifyViewUpdated();
    void notifyRestructure();

private:
    struct ItemInfo
    {
        ItemInfo()
            : ptr(NULL), pos() {}
        ItemInfo(QTreeWidgetItem* aPtr, const util::TreePos& aPos)
            : ptr(aPtr), pos(aPos) {}
        QTreeWidgetItem* ptr;
        util::TreePos pos;
    };

    // from QTreeWidget
    virtual void paintEvent(QPaintEvent* aEvent);
    virtual void showEvent(QShowEvent* aEvent);
    virtual void dragMoveEvent(QDragMoveEvent* aEvent);
    virtual void dropEvent(QDropEvent* aEvent);
    virtual void rowsInserted(const QModelIndex& aParent, int aStart, int aEnd);
    virtual void scrollContentsBy(int aDx, int aDy);
    virtual void resizeEvent(QResizeEvent* aEvent);

    void createTree(core::ObjectTree* aTree);
    void addItemRecursive(QTreeWidgetItem* aItem, core::ObjectNode* aNode);
    obj::Item* createFolderItem(core::ObjectNode& aNode);
    obj::Item* createFileItem(core::ObjectNode& aNode);
    void storeInsertion(
            const util::TreePos& aFrom,
            const util::TreePos& aTo,
            QTreeWidgetItem* aItem);
    QModelIndex cheatDragDropPos(QPoint& aPos);
    QPoint treeTopLeftPosition() const;
    int scrollHeight() const { return -treeTopLeftPosition().y(); }
    void endRenameEditor();
    int itemHeight(const core::ObjectNode& aNode) const;
    bool updateItemHeights(QTreeWidgetItem* aItem);

    void onTimeLineModified(core::TimeLineEvent&, bool);
    void onItemChanged(QTreeWidgetItem* aItem, int aColumn);
    void onItemClicked(QTreeWidgetItem* aItem, int aColumn);
    void onItemCollapsed(QTreeWidgetItem * aItem);
    void onItemExpanded(QTreeWidgetItem* aItem);
    void onItemSelectionChanged();
    void onContextMenuRequested(const QPoint& aPos);
    void onSlimActionTriggered(bool aIsTriggered);
    void onRenameActionTriggered(bool aIsTriggered);
    void onObjectActionTriggered(bool aIsTriggered);
    void onFolderActionTriggered(bool aIsTriggered);
    void onDeleteActionTriggered(bool aIsTriggered);

    ViaPoint& mViaPoint;
    GUIResources& mResources;
    util::LinkPointer<core::Project> mProject;
    util::SlotId mTimeLineSlot;

    bool mStoreInsert;
    std::vector<ItemInfo> mInserts;
    util::PlacePointer<cmnd::ScopedMacro> mMacroScope;
    core::ObjectTreeNotifier* mObjTreeNotifier;
    QModelIndex mDragIndex;
    QAbstractItemView::DropIndicatorPosition mDropIndicatorPos;

    QTreeWidgetItem* mActionItem;
    QAction* mSlimAction;
    QAction* mRenameAction;
    QAction* mObjectAction;
    QAction* mFolderAction;
    QAction* mDeleteAction;
};

} // namespace gui

#endif // GUI_OBJECTTREEWIDGET_H
